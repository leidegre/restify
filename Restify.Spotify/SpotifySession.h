
namespace Restify
{
    namespace Client
    {
        interface class ISpotifyMessage
        {
        public:
            void Invoke();
        };

        public ref class SpotifySession
        {
        private:
            sp_session_callbacks *_callbacks;
            sp_session_config *_config;
            sp_session *_session;
            
            Object ^_syncRoot;
            ConcurrentQueue<ISpotifyMessage ^> ^_queue;
            gcroot<SpotifySession ^> *_gcroot;

            waveform_api *_waveform;

            void EnsureSession() 
            { 
                if (_session == nullptr)
                    throw gcnew InvalidOperationException(L"You have to initialize Spotify first."); 
            }

        public:
            SpotifySession();
            ~SpotifySession();

            bool Initialize(array<Byte> ^appKey);

            void Login(String ^userName, String ^password);

        internal:
            void session_logged_in(sp_error error);
            void session_logged_out();
            void session_connection_error(sp_error error);
            void session_notify_main_thread();
            int session_music_delivery(const sp_audioformat *format, const void *frames, int num_frames);
            void session_end_of_track();
            void session_play_token_lost();

        public:
            event Action<SpotifyError> ^LoggedIn;
            event Action ^LoggedOut;
            event Action<SpotifyError> ^ConnectionError;
            event Action ^EndOfTrack;
            event Action ^PlayTokenLost;

        private:
            bool _hasMessageLoopNotification;
            void NotifyMessageLoop();
            
            bool _stopMessageLoop;

            static int _messageLoopThreadId;

        public:
            static bool HasAccess()
            {
                return _messageLoopThreadId == GetCurrentThreadId();
            }

            static void EnsureAccess()
            {
                if (!HasAccess())
                    throw gcnew InvalidOperationException(L"The Spotify API has to be called through a single thread. If you need to access the Spotify API from a different thread, use Post or PostSynchronized.");
            }

            void Post(Action ^action);
            void PostSynchronized(Action ^action);

            void RunMessageLoop();
            void StopMessageLoop();

        };


    }
}