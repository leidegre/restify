#pragma once

namespace Restify
{
    namespace Client
    {
        public ref class SpotifySessionConfiguration
        {
        public:
            property array<Byte> ^ApplicationKey;
            property String ^CacheLocation;
            property String ^SettingsLocation;

            SpotifySessionConfiguration()
            {
                CacheLocation = L"tmp";
                SettingsLocation = L"tmp";
            }
        };

        public ref class SpotifySession
        {
        private:
            Delegate ^_callback;
            Delegate ^_loggedIn;
            Delegate ^_metadataUpdated;

            sp_session *_session;
            
            Object ^_syncRoot;
            ConcurrentQueue<ISpotifyMessage ^> ^_queue;

            waveform_api *_waveform;

        public:
            SpotifySession(SpotifySessionConfiguration ^configuration);
            ~SpotifySession();

            void Login(String ^userName, String ^password, bool rememberMe);
            String ^GetMe();
            void LoginMe();
            void ForgetMe();
            
            bool LoadTrack(SpotifyTrack ^track);
            void PlayTrack(bool play);
            void SeekTrack(int offset);
            void UnloadTrack();
            bool PrefetchTrack(SpotifyTrack ^track);

        internal:
            void session_logged_in(sp_error error);
            void session_logged_out();
            void session_connection_error(sp_error error);
            void session_notify_main_thread();
            void session_metadata_updated(sp_session *session);
            
            int session_music_delivery(const sp_audioformat *format, const void *frames, int num_frames);
            void session_end_of_track();
            
            void session_play_token_lost();

        public:
            event Action<SpotifyError> ^LoggedIn;
            event Action ^LoggedOut;
            event Action<SpotifyError> ^ConnectionError;
            event Action ^MetadataUpdated;
            event Action ^EndOfTrack;
            event Action ^PlayTokenLost;

        private:
            bool _hasMessageLoopNotification;
            void NotifyMessageLoop();
            
            bool _stopMessageLoop;

        internal:
            void Post(ISpotifyMessage ^msg);
            void PostSynchronized(ISpotifyMessage ^msg);

        public:
            void Post(Action ^action);
            void PostSynchronized(Action ^action);

            void RunMessageLoop();
            void StopMessageLoop();

            void Search(SpotifySearchQuery ^search);

            ISpotifyMetaobject ^CreateMetaobject(String ^spotifyUri);
        };
    }
}