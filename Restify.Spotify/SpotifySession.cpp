
#include "restify.h"

namespace Restify
{
    namespace Spotify
    {
        void SP_CALLCONV Spotify_logged_in(sp_session *session, sp_error error)
        {
            trace("Spotify_logged_in: %s\r\n", sp_error_message(error));
            auto s = GetSpotifySession(session);
            s->OnLoggedIn(gcnew SpotifyEventArgs(error));
        }

        void SP_CALLCONV Spotify_logged_out(sp_session *session)
        {
            trace("Spotify_logged_out\r\n");
        }

        void SP_CALLCONV Spotify_connection_error(sp_session *session, sp_error error)
        {
            trace("Spotify_connection_error: %s\r\n", sp_error_message(error));
        }

        void SP_CALLCONV Spotify_notify_main_thread(sp_session *session)
        {
            trace("Spotify_notify_main_thread\r\n");
            GetSpotifySession(session)->Notify();
        }

        void SP_CALLCONV Spotify_metadata_updated(sp_session *session)
        {
            trace("Spotify_metadata_updated\r\n");
        }

        void SP_CALLCONV Spotify_end_of_track(sp_session *session)
        {
            trace("Spotify_end_of_track\r\n");
        }

        void SP_CALLCONV Spotify_play_token_lost(sp_session *session)
        {
            trace("Spotify_play_token_lost\r\n");
        }

        //
        // SIMPLE AUDIO RING BUFFER
        //
        // (audio will stutter if the buffer is too small, 
        //  increasing the chunk size might help with that)
        // the waveform API isn't very capable but it's simple to use
        //
        static const int    audio_buffer_chunk_count    = 256;
        static const int    audio_buffer_chunk_size     = sizeof(WAVEHDR) + 8 * 1024;
        void               *audio_buffer;
        unsigned            audio_buffer_head;
        unsigned            audio_buffer_tail;

        int SP_CALLCONV Spotify_music_delivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames)
        {
            MMRESULT result;

            if (num_frames == 0)
                return 0; // audio discontinuity, do nothing

            static LARGE_INTEGER freq;
            static BOOL _freq = QueryPerformanceFrequency(&freq);

            LARGE_INTEGER t0, t;
            QueryPerformanceCounter(&t0);

            auto s = GetSpotifySession(session);

            if (s->_waveOut == nullptr)
            {
                WAVEFORMATEX fmt;
                RtlZeroMemory(&fmt, sizeof(WAVEFORMATEX));
                fmt.wFormatTag = WAVE_FORMAT_PCM;
                fmt.nChannels = format->channels;
                fmt.nSamplesPerSec = format->sample_rate;
                fmt.nAvgBytesPerSec = format->sample_rate * 2 * format->channels;
                fmt.nBlockAlign = 2 * format->channels;
                fmt.wBitsPerSample = 16;
                fmt.cbSize = sizeof(WAVEFORMATEX);
                
                HWAVEOUT waveOut;
                if (waveOutOpen(&waveOut, WAVE_MAPPER, &fmt, NULL, NULL, CALLBACK_NULL) == MMSYSERR_NOERROR)
                {
                    if (s->_waveOut != nullptr)
                        free(audio_buffer); // if the _waveOut field has been set before, there is an audio buffer to be freed

                    audio_buffer        = malloc(audio_buffer_chunk_count * audio_buffer_chunk_size);
                    audio_buffer_head   = 0;
                    audio_buffer_tail   = 0;
                    
                    s->_waveOut = waveOut;
                }
            }

            PVOID p, pBuffer;
            WAVEHDR *wave;

            int frameBufferCount = audio_buffer_head - audio_buffer_tail;
            if (frameBufferCount > 0)
            {
                // tail
                p           = (PCHAR)audio_buffer + (audio_buffer_tail % audio_buffer_chunk_count) * audio_buffer_chunk_size;
                pBuffer     = (PCHAR)p + sizeof(WAVEHDR);
                wave        = (WAVEHDR *)p;

                if (waveOutUnprepareHeader(s->_waveOut, wave, sizeof(WAVEHDR)) != WAVERR_STILLPLAYING)
                {
                    audio_buffer_tail++;
                }
            }

            // generic error 
            // (it's not an actual error but we wan't a single point of exit 
            // for this callback method to simplify the control flow)
            result = MMSYSERR_ERROR; 

            if (frameBufferCount < audio_buffer_chunk_count)
            {
                // head
                p       = (PCHAR)audio_buffer + (audio_buffer_head++ % audio_buffer_chunk_count) * audio_buffer_chunk_size;
                pBuffer = (PCHAR)p + sizeof(WAVEHDR);
                wave    = (WAVEHDR *)p;

                num_frames = min((audio_buffer_chunk_size - sizeof(WAVEHDR)) / (2 * format->channels), num_frames);

                RtlZeroMemory(wave, sizeof(WAVEHDR));
                wave->lpData = (LPSTR)pBuffer;
                wave->dwBufferLength = num_frames * (2 * format->channels);

                RtlCopyMemory(pBuffer, frames, num_frames * (2 * format->channels));

                if ((result = waveOutPrepareHeader(s->_waveOut, wave, sizeof(WAVEHDR))) == MMSYSERR_NOERROR)
                    result = waveOutWrite(s->_waveOut, wave, sizeof(WAVEHDR));
            }
            
            QueryPerformanceCounter(&t);

            trace("Spotify_music_delivery frameBufferCount: %5u dT=%6llu us\r\n", frameBufferCount, ((1000 * 1000) * (t.QuadPart - t0.QuadPart)) / freq.QuadPart);
            
            // bad returning 0, libSpotify will callback at a later time
            // we do this to throttle
            return result == MMSYSERR_NOERROR ? num_frames : 0;
        }

        gcroot<SpotifySession ^> GetSpotifySession(sp_session* s)
        {
            return *static_cast<gcroot<SpotifySession ^> *>(sp_session_userdata(s));
        }

        SpotifySession::SpotifySession(SynchronizationContext^ synchronizationContext)
        {
            _this = new gcroot<SpotifySession ^>(this);
            _sync = synchronizationContext;
            _callbacks = new sp_session_callbacks;
            _config = new sp_session_config;
            _session = nullptr;
            _is_stop_pending = false;

            // events
            _loggedInDelegate = gcnew SendOrPostCallback(this, &SpotifySession::OnLoggedInDelegate);
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

        void SpotifySession::Login(String ^user, String ^pass)
        {
            pin_ptr<Byte> userString = &StringToAnsi(user)[0];
            pin_ptr<Byte> passString = &StringToAnsi(pass)[0];
            trace("sp_session_login: %s:%s\r\n", (const char *)userString, (const char *)passString);
            sp_session_login(get_session(), (const char *)userString, (const char *)passString);
        }

        void SpotifySession::Logout()
        {
            sp_session_logout(get_session());
        }

        SpotifyPlaylistCollection^ SpotifySession::Playlists::get()
        {
            if (_pl == nullptr)
            {
                auto *pl_container = sp_session_playlistcontainer(get_session());
                if (pl_container != nullptr)
                {
                    _pl = gcnew SpotifyPlaylistCollection(this, pl_container);
                }
            }
            return _pl;
        }

        // <events>
        
        void SpotifySession::OnLoggedInDelegate(Object ^e)
        {
            LoggedIn(this, static_cast<SpotifyEventArgs ^>(e));
        }

        void SpotifySession::OnLoggedIn(SpotifyEventArgs ^e)
        {
            if (_sync != nullptr)
            {
                _sync->Post(_loggedInDelegate, e);
            }
            else
            {
                LoggedIn(this, e);
            }
        }

        // </events>

        // http://stackoverflow.com/questions/143063/eventwaithandle-behavior-for-pthread-cond-t

        void SpotifySession::Notify()
        {
            Monitor::Enter(this);
            _notify_do = true;
            Monitor::Pulse(this);
            Monitor::Exit(this);
        }

        void SpotifySession::Shutdown()
        {
            _is_stop_pending = true;
        }

        void SpotifySession::Run()
        {
            // NOTE:
            // the main loop might hang if the cache is ever corrupted
            // deleting the tmp folder can resolve that problem

            Monitor::Enter(this);

            for (int next_timeout = 0;;)
            {
                if (next_timeout == 0)
                    while (!_notify_do)
                        Monitor::Wait(this);
                else
                    Monitor::Wait(this, next_timeout);
                
                _notify_do = false;
                Monitor::Exit(this);
                
                RunLockStep();
                
                do 
                {
                    sp_session_process_events(_session, &next_timeout);
                } 
                while (next_timeout == 0);

                if (_is_stop_pending)
                    break;

                Monitor::Enter(this);
            }
        }

        void SpotifySession::RunLockStep()
        {
            // Check pending stuff
            if (_track != nullptr && sp_track_is_available(_session, _track->get_track()))
            {
                sp_session_player_unload(_session);
                sp_error error;
                error = sp_session_player_load(_session, _track->get_track());
                if (error != SP_ERROR_OK)
                {
                    throw gcnew SpotifyException(error);
                }
                sp_session_player_play(_session, 1);
                _track = nullptr;
            }
        }

        // Stuff you might wanna do

        void SpotifySession::Play(SpotifyTrack ^track)
        {
            _track = track;
        }
    }
}