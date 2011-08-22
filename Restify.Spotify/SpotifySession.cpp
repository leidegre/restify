
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        void SP_CALLCONV Spotify_logged_in(sp_session *session, sp_error error)
        {
            trace("[%u] Spotify_logged_in: %s\r\n", GetCurrentThreadId(), sp_error_message(error));
            auto s = GetSpotifySession(session);
            s->_loggedInError = error;
            s->_loggedInEvent->Set();
        }

        void SP_CALLCONV Spotify_logged_out(sp_session *session)
        {
            trace("[%u] Spotify_logged_out\r\n", GetCurrentThreadId());
        }

        void SP_CALLCONV Spotify_connection_error(sp_session *session, sp_error error)
        {
            trace("[%u] Spotify_connection_error: %s\r\n", GetCurrentThreadId(), sp_error_message(error));
        }

        void SP_CALLCONV Spotify_notify_main_thread(sp_session *session)
        {
            trace("[%u] Spotify_notify_main_thread\r\n", GetCurrentThreadId());
            GetSpotifySession(session)->Notify();
        }

        void SP_CALLCONV Spotify_metadata_updated(sp_session *session)
        {
            trace("[%u] Spotify_metadata_updated\r\n", GetCurrentThreadId());
        }

        void SP_CALLCONV Spotify_end_of_track(sp_session *session)
        {
            trace("[%u] Spotify_end_of_track\r\n", GetCurrentThreadId());
        }

        void SP_CALLCONV Spotify_play_token_lost(sp_session *session)
        {
            trace("[%u] Spotify_play_token_lost\r\n", GetCurrentThreadId());
        }

        gcroot<SpotifySession ^> GetSpotifySession(sp_session* s)
        {
            return *static_cast<gcroot<SpotifySession ^> *>(sp_session_userdata(s));
        }

        SpotifySession::SpotifySession()
            : _syncRoot(gcnew Object())
        {
            _this = new gcroot<SpotifySession ^>(this);
            _callbacks = new sp_session_callbacks;
            _config = new sp_session_config;
            _session = nullptr;
            _is_stop_pending = false;
            _synq = gcnew ConcurrentQueue<ISpotifyAction ^>();

            // wait primitives
            _loggedInEvent = gcnew ManualResetEventSlim(false);
        }

        SpotifySession::~SpotifySession()
        {
            if (_this)
            {
                delete _this;
                _this = nullptr;
            }
            if (_callbacks)
            {
                delete _callbacks;
                _callbacks = nullptr;
            }
            if (_config)
            {
                delete _config;
                _config = nullptr;
            }
        }

        void SpotifySession::Initialize(array<Byte> ^key)
        {
            sp_error err;

            if (_session)
                return;
            
            RtlZeroMemory(_config, sizeof(sp_session_config));
            _config->api_version = SPOTIFY_API_VERSION;
            _config->cache_location = "tmp";
            _config->settings_location = "tmp";
            pin_ptr<Byte> pKey = &key[0];
            _config->application_key = pKey;
            _config->application_key_size = key->Length;
            _config->user_agent = "RESTify";
            _config->userdata = _this;
            _config->callbacks = _callbacks;
            
            RtlZeroMemory(_callbacks, sizeof(sp_session_callbacks));
            _callbacks->logged_in = &Spotify_logged_in;
            _callbacks->logged_out = &Spotify_logged_out;
            _callbacks->connection_error = &Spotify_connection_error;
            _callbacks->notify_main_thread = &Spotify_notify_main_thread;
            _callbacks->music_delivery = &Spotify_music_delivery;
            _callbacks->metadata_updated = &Spotify_metadata_updated;
            _callbacks->end_of_track = &Spotify_end_of_track;
            _callbacks->play_token_lost = &Spotify_play_token_lost;

            sp_session *session;

            err = sp_session_create(_config, &session);

            if (SP_ERROR_OK != err)
            {
                throw gcnew SpotifyException(err);
            }

            _session = session;
        }

        bool SpotifySession::Login(String ^user, String ^pass)
        {
            LOCK(this); // no concurrency
            _loggedInEvent->Reset();
            Do(gcnew SpotifyLoginAction(user, pass));
            if (!_loggedInEvent->Wait(DefaultTimeout))
            {
                throw gcnew TimeoutException();
            }
            return _loggedInError == SP_ERROR_OK;
        }

        void SpotifySession::Logout()
        {
            sp_session_logout(get_session());
        }

        List<SpotifyPlaylist ^> ^SpotifySession::GetPlaylistCollection()
        {
            LOCK(this);
            if (_pl_container == nullptr)
            {
                _getPlaylistCollection = gcnew SpotifyGetPlaylistCollectionAction();
                Do(_getPlaylistCollection);
                if (!_getPlaylistCollection->Wait())
                {
                    throw gcnew TimeoutException();
                }
                _getPlaylistCollection = nullptr;
            }
            return _pl_container->ToList();
        }
        
        // Stuff you might wanna do
        
        void SpotifySession::Shutdown()
        {
            _is_stop_pending = true;
            Notify();
        }

        void SpotifySession::Play(SpotifyTrack ^track)
        {
            //_track = track;
        }

        // THIS IS BASED ON THE JUKEBOX SAMPLE PROVIDED IN THE libspotify DISTRIBUTION
        // THE MANAGED EQUIVALENT OF THIS USES THE Monitor ref class AND IT SYNCHRONIZES USING
        // A PRIVATE SYNCHRONIZATION OBJECT
        //  http://stackoverflow.com/questions/143063/eventwaithandle-behavior-for-pthread-cond-t

        void SpotifySession::Notify()
        {
            Monitor::Enter(_syncRoot);
            _notify_do = true;
            Monitor::Pulse(_syncRoot);
            Monitor::Exit(_syncRoot);
        }

        void SpotifySession::Run()
        {
            // NOTE:
            // the main loop might hang if the cache is ever corrupted
            // deleting the tmp folder can resolve that problem

            Monitor::Enter(_syncRoot);

            for (int next_timeout = 0;;)
            {
                if (next_timeout == 0)
                    while (!_notify_do)
                        Monitor::Wait(_syncRoot);
                else
                    Monitor::Wait(_syncRoot, next_timeout);
                
                _notify_do = false;
                Monitor::Exit(_syncRoot);
                
                RunLockStep();
                
                do 
                {
                    sp_session_process_events(_session, &next_timeout);
                } 
                while (next_timeout == 0);

                if (_is_stop_pending)
                    break;

                Monitor::Enter(_syncRoot);
            }
        }

        void SpotifySession::RunLockStep()
        {
            trace("[%u] RunLockStep\r\n", GetCurrentThreadId());

            ISpotifyAction ^op;
            while (_synq->TryDequeue(op))
            {
                op->Do(this);
            }

            // Check pending stuff
            //if (_track != nullptr && sp_track_is_available(_session, _track->get_track()))
            //{
            //    sp_session_player_unload(_session);
            //    sp_error error;
            //    error = sp_session_player_load(_session, _track->get_track());
            //    if (error != SP_ERROR_OK)
            //    {
            //        throw gcnew SpotifyException(error);
            //    }
            //    sp_session_player_play(_session, 1);
            //    _track = nullptr;
            //}
        }
    }
}