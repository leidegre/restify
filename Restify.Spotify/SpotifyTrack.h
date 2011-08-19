#pragma once

namespace Restify
{
    namespace Spotify
    {
        public ref class SpotifyTrack
        {
        private:
            sp_track *_track;

        internal:
            SpotifyTrack(sp_track *track)
                : _track(track)
            {
            }

            sp_track *get_track() { return _track; }

        public:
            property bool IsLoaded
            {
                bool get()
                {
                    return sp_track_is_loaded(_track);
                }
            }

            property String ^Title
            {
                String ^get()
                {
                    return gcnew String(sp_track_name(_track));
                }
            }

            property TimeSpan Length
            {
                TimeSpan get()
                {
                    return TimeSpan(TimeSpan::TicksPerMillisecond * sp_track_duration(_track));
                }
            }

        };
    }
}