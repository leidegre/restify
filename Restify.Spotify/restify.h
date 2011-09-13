#pragma once

#include <libspotify/api.h>

#include <windows.h>
#include <stdio.h>

#define trace(fmt, ...) \
    { char __buf[1024]; _snprintf_s(__buf, 1024, fmt, __VA_ARGS__); OutputDebugStringA(__buf); printf("%s", __buf); }

#include "audio.h"

#include <vcclr.h> // gcroot
#include <msclr/lock.h> // lock

// <gcroot>

// internal helper functions to allow managed objects 
// to be passing around with callbacks

template<typename T>
inline
gcroot<T> *gcalloc(T value)
{
    return new gcroot<T>(value);
}

template<typename T>
inline
gcroot<T> gcget(void *p)
{
    return *static_cast<gcroot<T> *>(p);
}

template<typename T>
inline
void gcfree(void *p)
{
    delete static_cast<gcroot<T> *>(p);
}

generic<typename A, typename B>
ref class Pair
{
public:
    initonly A a;
    initonly B b;
    Pair(A a, B b)
        : a(a)
        , b(b)
    {
    }
};

template<typename A, typename B>
inline
Pair<A, B> ^gcpair(A a, B b)
{
    return gcnew Pair<A, B>(a, b);
}

// </gcroot>

#define CONCAT(x, y) x ## y

#define LOCK(object) \
    msclr::lock CONCAT(lock__, __LINE__)(object)

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace System::Collections::Concurrent;

using namespace System::Threading;

template<typename T>
inline
T dtof(Delegate ^d)
{
    using namespace System::Runtime::InteropServices;
    GCHandle::Alloc(d); // prevent delegate from being garbage collected (this is leaky, yes, but by design)
    IntPtr fp = Marshal::GetFunctionPointerForDelegate(d);
    return static_cast<T>(fp.ToPointer());
}

//
// libspotify thread safety helper functions 
// (NOTE: you can't run more than one instance at a time)
//
void sp_set_thread_access();
bool sp_has_thread_access();

inline
void sp_get_thread_access()
{
    if (!sp_has_thread_access())
        throw gcnew InvalidOperationException(L"The Spotify API has to be called through a single thread. If you need to access the Spotify API from a different thread, use Post or PostSynchronized to do so.");
}

//
//  Converts a managed Unicode character string to a c-style null-terminated UTF-8 `array<Byte> ^`
//
//  to pass the string to libspotify:
//      pin_ptr<Byte> p = &stringify(...)[0];
//  and it works because stringify will always return an array<Byte> ^ of length 1 
//  (the empty, null terminated string)
//  you then cast that pinned managed memory to the desirable pointer type (const char *)p
//  and pass it to libspotify
array<Byte> ^stringify(String ^s);
String ^unstringify(const char *s);

ref class IntPtrEqualityComparer : IEqualityComparer<IntPtr>
{
public:
    virtual int GetHashCode(IntPtr x)
    {
        return x.ToInt32();
    }
    
    virtual bool Equals(IntPtr x, IntPtr y)
    {
        return x.ToInt32() == y.ToInt32();
    }
};

namespace Restify
{
    namespace Client
    {
        public enum class SpotifyError {
            SP_ERROR_OK                        = 0,  ///< No errors encountered
	        SP_ERROR_BAD_API_VERSION           = 1,  ///< The library version targeted does not match the one you claim you support
	        SP_ERROR_API_INITIALIZATION_FAILED = 2,  ///< Initialization of library failed - are cache locations etc. valid?
	        SP_ERROR_TRACK_NOT_PLAYABLE        = 3,  ///< The track specified for playing cannot be played
	        SP_ERROR_BAD_APPLICATION_KEY       = 5,  ///< The application key is invalid
	        SP_ERROR_BAD_USERNAME_OR_PASSWORD  = 6,  ///< Login failed because of bad username and/or password
	        SP_ERROR_USER_BANNED               = 7,  ///< The specified username is banned
	        SP_ERROR_UNABLE_TO_CONTACT_SERVER  = 8,  ///< Cannot connect to the Spotify backend system
	        SP_ERROR_CLIENT_TOO_OLD            = 9,  ///< Client is too old, library will need to be updated
	        SP_ERROR_OTHER_PERMANENT           = 10, ///< Some other error occurred, and it is permanent (e.g. trying to relogin will not help)
	        SP_ERROR_BAD_USER_AGENT            = 11, ///< The user agent string is invalid or too long
	        SP_ERROR_MISSING_CALLBACK          = 12, ///< No valid callback registered to handle events
	        SP_ERROR_INVALID_INDATA            = 13, ///< Input data was either missing or invalid
	        SP_ERROR_INDEX_OUT_OF_RANGE        = 14, ///< Index out of range
	        SP_ERROR_USER_NEEDS_PREMIUM        = 15, ///< The specified user needs a premium account
	        SP_ERROR_OTHER_TRANSIENT           = 16, ///< A transient error occurred.
	        SP_ERROR_IS_LOADING                = 17, ///< The resource is currently loading
	        SP_ERROR_NO_STREAM_AVAILABLE       = 18, ///< Could not find any suitable stream to play
	        SP_ERROR_PERMISSION_DENIED         = 19, ///< Requested operation is not allowed
	        SP_ERROR_INBOX_IS_FULL             = 20, ///< Target inbox is full
	        SP_ERROR_NO_CACHE                  = 21, ///< Cache is not enabled
	        SP_ERROR_NO_SUCH_USER              = 22, ///< Requested user does not exist
	        SP_ERROR_NO_CREDENTIALS            = 23, ///< No credentials are stored
        };

        ref class SpotifyTrack;
        ref class SpotifyAlbum;
        ref class SpotifyArtist;
        //ref class SpotifyPlaylist;
        //ref class SpotifyPlaylistCollection;
        ref class SpotifySession;

        interface class ISpotifyMessage
        {
        public:
            void Invoke();
        };
    }
}

//#include "SpotifyLink.h"
#include "SpotifyMetaobject.h"
#include "SpotifyException.h"
#include "SpotifySearch.h"
//#include "SpotifyTrack.h"
//#include "SpotifyPlaylist.h"
//#include "SpotifyPlaylistCollection.h"
#include "SpotifySession.h"
