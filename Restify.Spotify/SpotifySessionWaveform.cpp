
#include "restify.h"

namespace Restify
{
    namespace Client
    {
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
    }
}