#pragma once

#include <libspotify/api.h>

#include <windows.h>
#include <stdio.h>

#define trace(fmt, ...) \
    { char __buf[1024]; _snprintf_s(__buf, 1024, fmt, __VA_ARGS__); OutputDebugStringA(__buf); printf("%s", __buf); }

#include "waveform.h"

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
//      pin_ptr<Byte> p = &Stringify(...)[0];
//  and it works because Stringify will always return an array<Byte> ^ of length 1 
//  (the empty, null terminated string)
//  you then cast that pinned managed memory to the desirable pointer type (const char *)p
//  and pass it to libspotify
array<Byte> ^Stringify(String ^s);
String ^Unstringify(const char *s);

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
            Ok = SP_ERROR_OK,
            BadApiVersion = SP_ERROR_BAD_API_VERSION,
            ApiInitializationFailed = SP_ERROR_API_INITIALIZATION_FAILED,
            TrackNotPlayable = SP_ERROR_TRACK_NOT_PLAYABLE,
            ResourceNotLoaded = SP_ERROR_RESOURCE_NOT_LOADED,
            BadApplicationKey = SP_ERROR_BAD_APPLICATION_KEY,
            BadUsernameOrPassword = SP_ERROR_BAD_USERNAME_OR_PASSWORD,
            UserBanned = SP_ERROR_USER_BANNED,
            UnableToContactServer = SP_ERROR_UNABLE_TO_CONTACT_SERVER,
            ClientTooOld = SP_ERROR_CLIENT_TOO_OLD,
            OtherPermanent = SP_ERROR_OTHER_PERMANENT,
            BadUserAgent = SP_ERROR_BAD_USER_AGENT,
            MissingCallback = SP_ERROR_MISSING_CALLBACK,
            InvalidIndata = SP_ERROR_INVALID_INDATA,
            IndexOutOfRange = SP_ERROR_INDEX_OUT_OF_RANGE,
            UserNeedsPremium = SP_ERROR_USER_NEEDS_PREMIUM,
            OtherTransient = SP_ERROR_OTHER_TRANSIENT,
            IsLoading = SP_ERROR_IS_LOADING,
            NoStreamAvailable = SP_ERROR_NO_STREAM_AVAILABLE,
            PermissionDenied = SP_ERROR_PERMISSION_DENIED,
            InboxIsFull = SP_ERROR_INBOX_IS_FULL,
            NoCache = SP_ERROR_NO_CACHE,
            NoSuchUser = SP_ERROR_NO_SUCH_USER
        };

        ref class SpotifyTrack;
        ref class SpotifyAlbum;
        ref class SpotifyArtist;
        ref class SpotifyPlaylist;
        ref class SpotifyPlaylistCollection;
        ref class SpotifySession;

        interface class ISpotifyMessage
        {
        public:
            void Invoke();
        };
    }
}

#include "SpotifyLink.h"
#include "SpotifyException.h"
#include "SpotifySearch.h"
#include "SpotifyTrack.h"
#include "SpotifyPlaylist.h"
#include "SpotifyPlaylistCollection.h"
#include "SpotifySession.h"
