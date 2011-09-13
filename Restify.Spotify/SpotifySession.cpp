
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        delegate void sp_action();
        delegate void sp_action_sp_session_(sp_session *session);
        delegate void sp_action_sp_error_(sp_error error);

        typedef void (__stdcall *sp_interop)();
        typedef void (__stdcall *sp_interop_session)(sp_session *session);
        typedef void (__stdcall *sp_interop_error)(sp_error error);

        struct re_userdata {
            gcroot<SpotifySession ^> *session;
            waveform_api *waveform;
            sp_interop notify_main_thread;
            sp_interop_error logged_in;
        };
    }
        
    // audio
    namespace Client
    {

        int SP_CALLCONV re_session_music_delivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames)
        {
            re_userdata *userdata = static_cast<re_userdata *>(sp_session_userdata(session));
            return waveform_music_delivery(userdata->waveform, format, frames, num_frames);
        }

        void SP_CALLCONV re_session_get_audio_buffer_stats(sp_session *session, sp_audio_buffer_stats *stats)
        {
            re_userdata *userdata = static_cast<re_userdata *>(sp_session_userdata(session));
            stats->stutter = 0; // wow, are we really that good?
            stats->samples = waveform_get_sample_count(userdata->waveform);
        }

        void SP_CALLCONV re_session_start_playback(sp_session *session)
        {
            re_userdata *userdata = static_cast<re_userdata *>(sp_session_userdata(session));
            waveform_play(userdata->waveform);
        }

        void SP_CALLCONV re_session_stop_playback(sp_session *session)
        {
            re_userdata *userdata = static_cast<re_userdata *>(sp_session_userdata(session));
            waveform_pause(userdata->waveform);
        }

        void SpotifySession::Flush()
        {
            sp_get_thread_access();
            re_userdata *userdata = static_cast<re_userdata *>(sp_session_userdata(_session));
            waveform_reset(userdata->waveform);
        }
    }
    
    namespace Client
    {
        void SP_CALLCONV sp_session_notify_main_thread(sp_session *session)
        {
            re_userdata *userdata = static_cast<re_userdata *>(sp_session_userdata(session));
            userdata->notify_main_thread();
        }

        void SpotifySession::session_notify_main_thread()
        {
            NotifyMessageLoop();
        }

        void SpotifySession::session_metadata_updated(sp_session *session)
        {
            MetadataUpdated();
        }

        gcroot<SpotifySession ^> GetSpotifySession(sp_session* session)
        {
            // NOTE: this is a thread-safe operation (or it has to be, otherwise very little would be possible)
            re_userdata *userdata = static_cast<re_userdata *>(sp_session_userdata(session));
            return *userdata->session;
        }

        void SP_CALLCONV sp_session_logged_in(sp_session *session, sp_error error)
        {
            re_userdata *userdata = static_cast<re_userdata *>(sp_session_userdata(session));
            userdata->logged_in(error);
            //GetSpotifySession(session)->session_logged_in(error);
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

        SpotifySession::SpotifySession(SpotifySessionConfiguration ^configuration)
            : _syncRoot(gcnew Object())
            , _callback(gcnew sp_action(this, &SpotifySession::session_notify_main_thread))
            , _loggedIn(gcnew sp_action_sp_error_(this, &SpotifySession::session_logged_in))
            , _metadataUpdated(gcnew sp_action_sp_session_(this, &SpotifySession::session_metadata_updated))
            , _queue(gcnew ConcurrentQueue<ISpotifyMessage ^>())
        {
            if (configuration == nullptr)
                throw gcnew ArgumentNullException("configuration");

            if (configuration->ApplicationKey == nullptr)
                throw gcnew ArgumentException("Application key cannot be null.", "configuration");

            auto root = gcalloc(this);
            
            sp_session_callbacks *callbacks = new sp_session_callbacks;
            RtlZeroMemory(callbacks, sizeof(sp_session_callbacks));
            callbacks->notify_main_thread = &sp_session_notify_main_thread;
            callbacks->logged_in = &sp_session_logged_in;
            callbacks->logged_out = &sp_session_logged_out;
            callbacks->connection_error = &sp_session_connection_error;
            // audio
            callbacks->music_delivery = &re_session_music_delivery;
            callbacks->get_audio_buffer_stats = &re_session_get_audio_buffer_stats;
            callbacks->start_playback = &re_session_start_playback;
            callbacks->stop_playback = &re_session_stop_playback;
            callbacks->metadata_updated = dtof<sp_interop_session>(_metadataUpdated);
            callbacks->end_of_track = &sp_session_end_of_track;
            callbacks->play_token_lost = &sp_session_play_token_lost;

            sp_session_config *config = new sp_session_config;
            RtlZeroMemory(config, sizeof(sp_session_config));
            config->api_version = SPOTIFY_API_VERSION;
            pin_ptr<Byte> cache_location = &stringify(configuration->CacheLocation)[0];
            config->cache_location = (const char *)cache_location;
            pin_ptr<Byte> settings_location = &stringify(configuration->SettingsLocation)[0];
            config->settings_location = (const char *)settings_location;
            pin_ptr<Byte> pAppKey = &configuration->ApplicationKey[0];
            config->application_key = pAppKey;
            config->application_key_size = configuration->ApplicationKey->Length;
            config->user_agent = "RESTify";

            re_userdata *userdata = new re_userdata;
            userdata->session = root;
            waveform_init(&userdata->waveform);
            userdata->notify_main_thread = (sp_interop)System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(_callback).ToPointer();
            userdata->logged_in = (sp_interop_error)System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(_loggedIn).ToPointer();
            config->userdata = userdata;
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
            
            _session = session;
        }

        SpotifySession::~SpotifySession()
        {
            if (_session != nullptr)
            {
                sp_session_release(_session);
                _session = nullptr;
            }
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
            pin_ptr<Byte> pUserName = &stringify(userName)[0];
            pin_ptr<Byte> pPassword = &stringify(password)[0];
            sp_session_login(_session, (const char *)pUserName, (const char *)pPassword, false);
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
            if (track == nullptr)
                throw gcnew ArgumentNullException("track");
            sp_get_thread_access();
            sp_error error = sp_session_player_load(_session, track->get_track());
            trace("sp_session_player_load: %i (%u)\r\n", error == SP_ERROR_OK, error);
            return error == SP_ERROR_OK;
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
            pin_ptr<Byte> query = &stringify(search->Query)[0]; 
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

        ISpotifyMetaobject ^SpotifySession::CreateMetaobject(String ^spotifyUri)
        {
            if (spotifyUri == nullptr)
                throw gcnew ArgumentNullException(L"spotifyUri");

            if (!spotifyUri->StartsWith("spotify:"))
                throw gcnew ArgumentException(L"Spotify URIs starts with the 'spotify:' URI schema.", L"spotifyUri");

            pin_ptr<Byte> p = &stringify(spotifyUri)[0];

            sp_link *link = sp_link_create_from_string((const char *)p);
            if (!link)
                return nullptr;

            ISpotifyMetaobject ^obj = nullptr;

            switch (sp_link_type(link))
            {
            case SP_LINKTYPE_TRACK:
                {
                    sp_track *track = sp_link_as_track(link);
                    sp_track_add_ref(track);
                    obj = gcnew SpotifyTrack(track);
                }
                break;
            }

            sp_link_release(link);

            return obj;
        }
    }
}