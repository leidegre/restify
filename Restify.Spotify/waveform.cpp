
#include <libspotify/api.h>

#include <windows.h>
#include <stdio.h>

#include "audio.h"

#define trace(fmt, ...) \
    { char __buf[1024]; _snprintf_s(__buf, 1024, fmt, __VA_ARGS__); OutputDebugStringA(__buf); printf("%s", __buf); }

struct waveform_api
{
    static const int    audio_buffer_chunk_count    = 64;
    static const int    audio_buffer_chunk_size     = sizeof(WAVEHDR) + 8 * 1024;

    HWAVEOUT            audio_dev;
    void               *audio_buffer;
    
    unsigned            sample_size;
    unsigned            sample_count;
    
    unsigned            audio_buffer_head;
    unsigned            audio_buffer_tail;
};

bool waveform_init(waveform_api **waveform)
{
    if (!waveform)
        return false;

    waveform_api *w = new waveform_api;
    RtlZeroMemory(w, sizeof(waveform_api));
    
    w->audio_buffer = malloc(waveform_api::audio_buffer_chunk_count * waveform_api::audio_buffer_chunk_size);
    w->audio_buffer_head = 0;
    w->audio_buffer_tail = 0;

    *waveform = w;
    return true;
}

void waveform_reset(waveform_api *waveform)
{
    MMRESULT result = waveOutReset(waveform->audio_dev);
    trace("waveform_reset: %i (%u)\r\n", result == MMSYSERR_NOERROR, result);
}

int waveform_music_delivery(waveform_api *waveform, const sp_audioformat *format, const void *frames, int num_frames)
{
    MMRESULT result;

    if (!waveform)
        return 0; // no audio output

    if (num_frames == 0)
    {
        // a discontinuity has occurred (such as after a seek)
        waveform_reset(waveform);
        return 0; 
    }

    //static LARGE_INTEGER freq;
    //static BOOL _freq = QueryPerformanceFrequency(&freq);
    //LARGE_INTEGER t0, t;
    //QueryPerformanceCounter(&t0);
    
    if (!waveform->audio_dev)
    {
        // typically this would be 44.1KHz 16-bit stereo
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
            waveform->audio_dev = waveOut;
            waveform->sample_size = fmt.nBlockAlign;
        }
    }

    PVOID p, pBuffer;
    WAVEHDR *wave;

    int frameBufferCount = waveform->audio_buffer_head - waveform->audio_buffer_tail;
    if (frameBufferCount > 0)
    {
        // tail
        p           = (PCHAR)waveform->audio_buffer + (waveform->audio_buffer_tail % waveform_api::audio_buffer_chunk_count) * waveform_api::audio_buffer_chunk_size;
        pBuffer     = (PCHAR)p + sizeof(WAVEHDR);
        wave        = (WAVEHDR *)p;

        // this is fine, if the `waveOutUnprepareHeader` lags behind libspotify will back off
        // if we try to loop here to catch up there will be stutter (this we don't want)
        if (waveOutUnprepareHeader(waveform->audio_dev, wave, sizeof(WAVEHDR)) != WAVERR_STILLPLAYING)
        {
            unsigned sample_count = wave->dwBufferLength / (2 * format->channels);
            waveform->sample_count -= sample_count;
            waveform->audio_buffer_tail++;
        }
    }

    // set initial generic error 
    // it's not an actual error but we wan't a single point of exit to this function, 
    // this simplifies the control flow a bit
    result = MMSYSERR_ERROR;

    if (frameBufferCount < waveform_api::audio_buffer_chunk_count)
    {
        // head
        p       = (PCHAR)waveform->audio_buffer + (waveform->audio_buffer_head++ % waveform_api::audio_buffer_chunk_count) * waveform_api::audio_buffer_chunk_size;
        pBuffer = (PCHAR)p + sizeof(WAVEHDR);
        wave    = (WAVEHDR *)p;

        num_frames = min((waveform_api::audio_buffer_chunk_size - sizeof(WAVEHDR)) / (2 * format->channels), num_frames);

        RtlZeroMemory(wave, sizeof(WAVEHDR));
        wave->lpData = (LPSTR)pBuffer;
        wave->dwBufferLength = num_frames * (2 * format->channels);

        RtlCopyMemory(pBuffer, frames, num_frames * (2 * format->channels));

        if ((result = waveOutPrepareHeader(waveform->audio_dev, wave, sizeof(WAVEHDR))) == MMSYSERR_NOERROR)
            if ((result = waveOutWrite(waveform->audio_dev, wave, sizeof(WAVEHDR))) == MMSYSERR_NOERROR)
                waveform->sample_count += num_frames;
    }
            
    //QueryPerformanceCounter(&t);
    //trace("waveform_music_delivery frameBufferCount: %5u dT=%6llu us\r\n", frameBufferCount, ((1000 * 1000) * (t.QuadPart - t0.QuadPart)) / freq.QuadPart);
            
    //trace("music_delivery: sample_count=%u\r\n", waveform->sample_count);

    // returning 0 is bad libspotify will just callback at a later time 
    // but since the device is broken we cannot really do anything...
    return result == MMSYSERR_NOERROR ? num_frames : 0;
}

int waveform_get_sample_count(waveform_api *waveform)
{
    return (int)waveform->sample_count;
}

void waveform_pause(waveform_api *waveform)
{
    waveOutPause(waveform->audio_dev);
}

void waveform_play(waveform_api *waveform)
{
    waveOutRestart(waveform->audio_dev);
}

void waveform_destroy(waveform_api *waveform)
{
    waveOutClose(waveform->audio_dev);
}
