
namespace Restify
{
    namespace Client
    {
        interface class ISpotifyMessage
        {
        public:
            void Invoke();
        };

        // internal (used to zip the session together with value as a GCHandle, 
        // used with callbacks in libspotify)
        generic <typename T>
        ref class SpotifyTag<T>
        {
        private:
            SpotifySession ^_session; 
            T _value;

        public:
            property SpotifySession ^Session { SpotifySession ^get() { return _session; } }
            property T Value { T get() { return _value; } }

            SpotifySession(SpotifySession^ session, T value)
                : _session(session)
                , _value(value)
            {
            }

            static gcroot<SpotifySession<T> ^> *Alloc(T value)
            {
                return new gcroot<SpotifySession<T> ^>(gcnew SpotifySession(this, value));
            }

            static gcroot<SpotifySession<T> ^> ^Get(void *p)
            {
                return *static_cast<gcroot<SpotifySession<T> ^> *>(p);
            }

            static SpotifySession<T> ^Free(void *p)
            {
                gcroot<SpotifySession<T> ^> s = Get(p);
                delete static_cast<gcroot<SpotifySession<T> ^> *>(p);
                return s;
            }
        }

        public ref class SpotifySession
        {
        internal:
            generic <typename T>
            gcroot<SpotifySession<T> ^> *Alloc(T value)
            {
                return new gcroot<SpotifySession<T> ^>(gcnew SpotifySession(this, value));
            }

        private:
            gcroot<SpotifySession ^> *_gcroot;
            
            sp_session_callbacks *_callbacks;
            sp_session_config *_config;
            
            sp_session *_session;
            
            Object ^_syncRoot;
            ConcurrentQueue<ISpotifyMessage ^> ^_queue;

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
            
            bool HasAccess()
            {
                return _messageLoopThreadId == GetCurrentThreadId();
            }

            void EnsureAccess()
            {
                if (!HasAccess())
                    throw gcnew InvalidOperationException(L"The Spotify API has to be called through a single thread. If you need to access the Spotify API from a different thread, use Post or PostSynchronized.");
            }

        public:
            
            void Post(Action ^action);
            void PostSynchronized(Action ^action);

            void RunMessageLoop();
            void StopMessageLoop();

            SpotifyLink CreateLink(String ^link);
            
            SpotifyTrack ^CreateTrack(SpotifyLink link);

            void Search(SpotifySearch ^search);
        };


    }
}