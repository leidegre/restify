
#include "restify.h"

namespace Restify
{
    namespace Client
    {
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

        SpotifySession::SpotifySession(array<Byte> ^appKey)
            : _syncRoot(gcnew Object())
            , _queue(gcnew ConcurrentQueue<ISpotifyMessage ^>())
        {
            if (appKey == nullptr)
                throw gcnew ArgumentNullException("s");

            auto root = gcalloc(this);
            
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
            config->userdata = root;
            config->callbacks = callbacks;
            
            sp_session *session;

            sp_error err = sp_session_create(config, &session);
            if (SP_ERROR_OK != err)
            {
                delete callbacks;
                delete config;
                delete root;
                throw gcnew SpotifyException(err);
            }
            
            // Yeah, the constructor can leak memory however, in it's current state, 
            // libspotify doesn't support multiple sessions per process
            
            waveform_api *waveform;
            if (waveform_init(&waveform))
            {
                _waveform = waveform;
            }
            
            _session = session;
        }

        SpotifySession::~SpotifySession()
        {
        }

        //
        // Auth
        //

        void SpotifySession::Login(String ^userName, String ^password, bool rememberMe)
        {
            if (userName == nullptr)
                throw gcnew ArgumentNullException("userName");
            if (password == nullptr)
                throw gcnew ArgumentNullException("password");
            sp_get_thread_access();
            pin_ptr<Byte> pUserName = &Stringify(userName)[0];
            pin_ptr<Byte> pPassword = &Stringify(password)[0];
            sp_session_login(_session, (const char *)pUserName, (const char *)pPassword);
        }

        String ^SpotifySession::GetMe()
        {
            sp_get_thread_access();
            return nullptr;
        }

        void SpotifySession::LoginMe()
        {
            sp_get_thread_access();
        }

        void SpotifySession::ForgetMe()
        {
            sp_get_thread_access();
        }

        //
        // Playback
        //

        bool SpotifySession::LoadTrack(SpotifyTrack ^track)
        {
            sp_get_thread_access();
            return sp_session_player_load(_session, track->get_track()) == SP_ERROR_OK;
        }

        void SpotifySession::PlayTrack(bool play)
        {
            sp_get_thread_access();
            sp_session_player_play(_session, play);
        }

        void SpotifySession::SeekTrack(int offset)
        {
            sp_get_thread_access();
            sp_session_player_seek(_session, offset);
        }

        void SpotifySession::UnloadTrack()
        {
            sp_get_thread_access();
            sp_session_player_unload(_session);
        }

        bool SpotifySession::PrefetchTrack(SpotifyTrack ^track)
        {
            sp_get_thread_access();
            return sp_session_player_prefetch(_session, track->get_track()) == SP_ERROR_OK;
        }

        //
        // Message passing
        //
        
        ref class SpotifySynchronizedMessage : ISpotifyMessage
        {
            ISpotifyMessage ^_msg;
            ManualResetEventSlim ^_event;

        public:
            SpotifySynchronizedMessage(ISpotifyMessage ^msg)
                : _msg(msg)
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
                _msg->Invoke();
                _event->Set();
            }
        };
        
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
        
        void SpotifySession::Post(ISpotifyMessage ^msg)
        {
            if (msg == nullptr)
                throw gcnew ArgumentNullException("msg");
            if (!sp_has_thread_access()) // prevent posting from message loop to cause dead-lock
            {
                _queue->Enqueue(msg);
                NotifyMessageLoop();
            }
            else
            {
                msg->Invoke();
            }
        }
        
        void SpotifySession::PostSynchronized(ISpotifyMessage ^msg)
        {
            if (msg == nullptr)
                throw gcnew ArgumentNullException("msg");

            if (!sp_has_thread_access()) // prevent posting from message loop to cause dead-lock
            {
                // C++/CLI repurposes the delete keyword for managed classes, deconstructors implements IDisposable 
                // and let's you use the try/finally pattern to do deterministic cleanup (scoped)
                SpotifySynchronizedMessage ^syncMsg = nullptr; 
                try
                {
                    syncMsg = gcnew SpotifySynchronizedMessage(msg);
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
                msg->Invoke();
            }
        }

        void SpotifySession::Post(Action ^action)
        {
            if (action == nullptr)
                throw gcnew ArgumentNullException("action");

            Post(gcnew SpotifyMessage(action));
        }
        
        void SpotifySession::PostSynchronized(Action ^action)
        {
            if (action == nullptr)
                throw gcnew ArgumentNullException("action");

            PostSynchronized(gcnew SpotifyMessage(action));
        }
        
        void SP_CALLCONV sp_search_complete(sp_search *result, void *userdata)
        {
            auto s = gcget<Pair<SpotifySession ^, SpotifySearchQuery ^> ^>(userdata);
            s->a->Post(gcnew SpotifySearchResultMessage(s->b, result));
            gcfree<Pair<SpotifySession ^, SpotifySearchQuery ^> ^>(userdata);
        }

        void SpotifySession::Search(SpotifySearchQuery ^search)
        {
            if (search == nullptr)
                throw gcnew ArgumentNullException("search");
            sp_get_thread_access();
            pin_ptr<Byte> query = &Stringify(search->Query)[0]; 
            auto userdata = gcalloc(gcpair(this, search));
            sp_search_create(_session, (const char *)query, search->TrackOffset, search->TrackCount, search->AlbumOffset, search->AlbumCount, search->ArtistOffset, search->ArtistCount, &sp_search_complete, userdata);
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

            sp_set_thread_access();
            
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
                // Process external events
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