#pragma once

namespace Restify
{
    namespace Client
    {
        public ref class SpotifyException : Exception
        {
        public:
            SpotifyException(int errorCode)
                : Exception(gcnew String(sp_error_message((sp_error)errorCode)))
            {
            }
        };
    }
}