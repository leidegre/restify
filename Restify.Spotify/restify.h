#pragma once

#include <libspotify/api.h>

#include <windows.h>
#include <stdio.h>

#define trace(fmt, ...) \
    { char __buf[1024]; _snprintf_s(__buf, 1024, fmt, __VA_ARGS__); OutputDebugStringA(__buf); printf("%s", __buf); }

#include <vcclr.h> // gcroot
#include <msclr/lock.h> // lock


#define CONCAT(x, y) x ## y

#define LOCK(object) \
    msclr::lock CONCAT(lock__, __LINE__)(object)

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace System::Collections::Concurrent;
using namespace System::ComponentModel;

using namespace System::Threading;

using namespace System::Runtime::InteropServices;

// Convert a managed Unicode string to a null-terminated c-style ANSI string
array<Byte> ^StringToSpString(String ^s);
String ^SpStringToString(const char *s);

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
        ref class SpotifyPlaylist;
        ref class SpotifyPlaylistCollection;
        ref class SpotifySession;
    }
}

#include "SpotifyAsync.h"

#include "SpotifyObservable.h"

#include "SpotifyException.h"

#include "SpotifyTrack.h"
#include "SpotifyPlaylist.h"
#include "SpotifyPlaylistCollection.h"
#include "SpotifySession.h"
