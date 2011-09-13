#pragma once

namespace Restify
{
    namespace Client
    {
        public ref class SpotifyException : Exception
        {
        public:
            SpotifyException(int errorCode)
                : Exception(String::Format(L"{0} ({1})", unstringify(sp_error_message((sp_error)errorCode)), errorCode))
            {
            }
        };
    }
}