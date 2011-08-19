#pragma once

namespace Restify
{
    namespace Spotify
    {
        gcroot<SpotifySession ^> GetSpotifySession(sp_session* s);

        public ref class SpotifyEventArgs : EventArgs
        {
        private:
            sp_error _error;

        internal:
            SpotifyEventArgs(sp_error error)
                : _error(error)
            {
            }

        public:
            property bool HasError
            {
                bool get()
                {
                    return _error != SP_ERROR_OK;
                }
            }

            property String ^ErrorMessage
            {
                String ^get()
                {
                    return gcnew String(sp_error_message(_error));
                }
            }

        };

        public delegate void SpotifyEventHandler(SpotifySession^ sender, SpotifyEventArgs^ e);

        public ref class SpotifySession
        {
        private:
            gcroot<SpotifySession ^>* _this;

            sp_session_callbacks *_callbacks;
            sp_session_config *_config;
            sp_session *_session;

            bool _notify_do;
            
            void RunLockStep();
            
            bool _is_stop_pending;

            SendOrPostCallback ^_loggedInDelegate;

            SpotifyPlaylistCollection ^_pl;
            SpotifyTrack ^_track;

        internal:
            HWAVEOUT _waveOut;

            sp_session *get_session() 
            { 
                if (_session == nullptr)
                    throw gcnew InvalidOperationException("Must initialize Spotify session first.");
                return _session; 
            }
            
            SynchronizationContext^ _sync;

            void OnLoggedInDelegate(Object^ e);
            void OnLoggedIn(SpotifyEventArgs^ e);

            void Notify();

        public:
            event SpotifyEventHandler^ LoggedIn;

            SpotifySession(SynchronizationContext^ synchronizationContext);
            ~SpotifySession();

            void Initialize(array<Byte> ^key);

            property bool IsLoggedIn
            {
                bool get()
                {
                    if (_session != nullptr)
                        return sp_session_user(_session) != nullptr;
                    
                    return false;
                }
            }

            void Login(String^ user, String^ pass);

            void Logout();

            property SpotifyPlaylistCollection ^Playlists
            {
                SpotifyPlaylistCollection ^get();
            }

            void Run();

            void Shutdown();

            void Play(SpotifyTrack ^ track);
        };
    }
}