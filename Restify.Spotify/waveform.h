#pragma once

struct waveform_api;
bool waveform_init(waveform_api **waveform);
void waveform_reset(waveform_api *waveform);
void waveform_destroy(waveform_api *waveform);
int waveform_music_delivery(waveform_api *waveform, const sp_audioformat *format, const void *frames, int num_frames);
void waveform_pause(waveform_api *waveform);
void waveform_play(waveform_api *waveform);
