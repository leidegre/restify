
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        SpotifySession::SpotifySession()
            : _syncRoot(gcnew Object())
            , _queue(gcnew ConcurrentQueue<ISpotifyMessage ^>())
        {
            _gcroot = new gcroot<SpotifySession ^>(this);
        }

        SpotifySession::~SpotifySession()
        {
            if (_gcroot)
            {
                delete _gcroot;
                _gcroot = nullptr;
            }
        }
        
        //
        // Session callbacks
        //
        
        gcroot<SpotifySession ^> GetSpotifySession(sp_session* session)
        {
            // NOTE: this is a thread-safe operation (or it has to be, otherwise very little would be possible)
            return *static_cast<gcroot<SpotifySession ^> *>(sp_session_userdata(session));
        }

        void SP_CALLCONV sp_session_logged_in(sp_session *session, sp_error error)
        {
            GetSpotifySession(session)->session_logged_in(error);
        }

        void SpotifySession::session_logged_in(sp_error error)
        {
            LoggedIn((SpotifyError)error);
        }

        void SP_CALLCONV sp_session_logged_out(sp_session *session)
        {
            GetSpotifySession(session)->session_logged_out();
        }

        void SpotifySession::session_logged_out()
        {
            LoggedOut();
        }

        void SP_CALLCONV sp_session_connection_error(sp_session *session, sp_error error)
        {
            GetSpotifySession(session)->session_connection_error(error);
        }

        void SpotifySession::session_connection_error(sp_error error)
        {
            ConnectionError((SpotifyError)error);
        }

        void SP_CALLCONV sp_session_notify_main_thread(sp_session *session)
        {
            GetSpotifySession(session)->session_notify_main_thread();
        }

        void SpotifySession::session_notify_main_thread()
        {
            NotifyMessageLoop();
        }

        int SP_CALLCONV sp_session_music_delivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames)
        {
            return GetSpotifySession(session)->session_music_delivery(format, frames, num_frames);
        }

        int SpotifySession::session_music_delivery(const sp_audioformat *format, const void *frames, int num_frames)
        {
            return waveform_music_delivery(_waveform, format, frames, num_frames);
        }

        void SP_CALLCONV sp_session_end_of_track(sp_session *session)
        {
            GetSpotifySession(session)->session_end_of_track();
        }

        void SpotifySession::session_end_of_track()
        {
            EndOfTrack();
        }

        void SP_CALLCONV sp_session_play_token_lost(sp_session *session)
        {
            GetSpotifySession(session)->session_play_token_lost();
        }

        void SpotifySession::session_play_token_lost()
        {
            PlayTokenLost();
        }

        bool SpotifySession::Initialize(array<Byte> ^appKey)
        {
            Contract::Requires(appKey != nullptr);

            if (_session)
                throw gcnew InvalidOperationException(L"The Spotify API has already been initialized once.");
            
            waveform_api *waveform;
            if (waveform_init(&waveform))
            {
                _waveform = waveform;
            }

            sp_session_callbacks *callbacks = new sp_session_callbacks;
            RtlZeroMemory(callbacks, sizeof(sp_session_callbacks));
            callbacks->logged_in = &sp_session_logged_in;
            callbacks->logged_out = &sp_session_logged_out;
            callbacks->connection_error = &sp_session_connection_error;
            callbacks->notify_main_thread = &sp_session_notify_main_thread;
            callbacks->music_delivery = &sp_session_music_delivery;
            callbacks->end_of_track = &sp_session_end_of_track;
            callbacks->play_token_lost = &sp_session_play_token_lost;

            sp_session_config *config = new sp_session_config;
            RtlZeroMemory(config, sizeof(sp_session_config));
            config->api_version = SPOTIFY_API_VERSION;
            config->cache_location = "tmp";
            config->settings_location = "tmp";
            pin_ptr<Byte> pAppKey = &appKey[0];
            config->application_key = pAppKey;
            config->application_key_size = appKey->Length;
            config->user_agent = "RESTify";
            config->userdata = _gcroot;
            config->callbacks = callbacks;
            
            sp_session *session;

            sp_error err = sp_session_create(config, &session);
            if (SP_ERROR_OK != err)
            {
                delete callbacks;
                delete config;
                return false;
            }

            _callbacks = callbacks;
            _config = config;
            _session = session;
            return true;
        }

        void SpotifySession::Login(String ^userName, String ^password)
        {
            EnsureSession();
            EnsureAccess();
            Contract::Requires(userName != nullptr);
            Contract::Requires(password != nullptr);
            pin_ptr<Byte> pUserName = &Stringify(userName)[0];
            pin_ptr<Byte> pPassword = &Stringify(password)[0];
            sp_session_login(_session, (const char *)pUserName, (const char *)pPassword);
        }

        //
        // Message passing
        //

        ref class SpotifyMessage : ISpotifyMessage
        {
            Action ^_action;

        public:
            SpotifyMessage(Action ^action)
                : _action(action)
            {
            }

            virtual void Invoke()
            {
                _action();
            }
        };

        void SpotifySession::Post(Action ^action)
        {
            Contract::Requires(action != nullptr);
            if (!HasAccess()) // prevent posting from message loop to cause dead-lock
            {
                SpotifyMessage ^msg = gcnew SpotifyMessage(action);
                _queue->Enqueue(msg);
                NotifyMessageLoop();
            }
            else
            {
                action();
            }
        }
        
        ref class SpotifySynchronizedMessage : ISpotifyMessage
        {
            Action ^_action;
            ManualResetEventSlim ^_event;

        public:
            SpotifySynchronizedMessage(Action ^action)
                : _action(action)
            {
                _event = gcnew ManualResetEventSlim(false);
            }

            ~SpotifySynchronizedMessage()
            {
                delete _event;
            }

            void Wait()
            {
                _event->Wait();
            }

            virtual void Invoke()
            {
                _action();
                _event->Set();
            }
        };

        void SpotifySession::PostSynchronized(Action ^action)
        {
            Contract::Requires(action != nullptr);
            if (!HasAccess()) // prevent posting from message loop to cause dead-lock
            {
                // C++/CLI repurposes the delete keyword for managed classes, deconstructors implements IDisposable 
                // and let's you use the try/finally pattern to do deterministic cleanup (scoped)
                SpotifySynchronizedMessage ^syncMsg = nullptr; 
                try
                {
                    syncMsg = gcnew SpotifySynchronizedMessage(action);
                    _queue->Enqueue(syncMsg);
                    NotifyMessageLoop();
                    syncMsg->Wait();
                }
                finally
                {
                    if (syncMsg != nullptr)
                        delete syncMsg;
                }
            }
            else
            {
                action();
            }
        }

        //
        // Message loop
        //

        void SpotifySession::NotifyMessageLoop()
        {
            Monitor::Enter(_syncRoot);
            _hasMessageLoopNotification = true;
            Monitor::Pulse(_syncRoot);
            Monitor::Exit(_syncRoot);
        }

        void SpotifySession::RunMessageLoop()
        {
            if (!_session)
                throw gcnew InvalidOperationException(L"You have to initialize the Spotifi API before you can run the message loop.");

            SpotifySession::_messageLoopThreadId = GetCurrentThreadId();
            
            Monitor::Enter(_syncRoot);

            for (int next_timeout = 0;;)
            {
                if (next_timeout == 0)
                    while (!_hasMessageLoopNotification)
                        Monitor::Wait(_syncRoot);
                else
                    Monitor::Wait(_syncRoot, next_timeout);
                
                _hasMessageLoopNotification = false;
                Monitor::Exit(_syncRoot);
                
                //
                // Process managed events
                //
                ISpotifyMessage ^msg;
                while (SpotifySession::_queue->TryDequeue(msg))
                {
                    msg->Invoke();
                }
                
                //
                // Process libspotify events
                //
                do 
                {
                    sp_session_process_events(_session, &next_timeout);
                } 
                while (next_timeout == 0);

                if (_stopMessageLoop)
                    break;

                Monitor::Enter(_syncRoot);
            }
        }

        void SpotifySession::StopMessageLoop()
        {
            _stopMessageLoop = true;
            NotifyMessageLoop();
        }
    }
}