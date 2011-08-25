#pragma once

namespace Restify
{
    namespace Client
    {
        int SP_CALLCONV Spotify_music_delivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames);

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

        public ref class SpotifySessionConfiguration
        {
        public:
            property array<Byte> ^ApplicationKey;
            property String ^CacheLocation;
            property String ^SettingsLocation;
        };

        public ref class SpotifySession
        {
        public:
            static const int DefaultTimeout = 30 * 1000; 

        private:
            Object ^_syncRoot;

            gcroot<SpotifySession ^>* _this;

            sp_session_callbacks *_callbacks;
            sp_session_config *_config;
            sp_session *_session;

            bool _notify_do;
            
            void RunLockStep();
            
            bool _is_stop_pending;

            // all asynchronous work is synchronized through primitives in this queue
            ConcurrentQueue<ISpotifyAction ^> ^_synq;

        internal:
            // NOTE: these should only be accessed through session callbacks
            // these are basically callbacks slots
            sp_error _loggedInError;
            ManualResetEventSlim ^_loggedInEvent;
            
            SpotifyGetPlaylistCollectionAction ^_getPlaylistCollection;
            SpotifyPlaylistCollection ^_pl_container;
            
            sp_track *_trackToLoad, *_trackNowPlaying;

            ConcurrentQueue<String ^> ^_playQueue;

            void Spotify_end_of_track();

        internal:
            HWAVEOUT _waveOut;

            sp_session *get_session() 
            { 
                if (_session == nullptr)
                    throw gcnew InvalidOperationException("Must initialize Spotify session first.");
                return _session; 
            }
            
            void Do(ISpotifyAction ^op)
            {
                _synq->Enqueue(op);
                Notify();
            }

            void Notify();

        public:
            SpotifySession();
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

            bool Login(String ^user, String ^pass);

            void Logout();

            List<SpotifyPlaylist ^> ^GetPlaylistCollection();

            void Run();

            void Shutdown();

            void Play(SpotifyTrack ^track);
            void PlayLink(String ^trackId);

            void EnqueueLink(String ^trackId);
        };
    }
}