
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        void SpotifyTrack::Nudge()
        {
            if (sp_track_is_loaded(_track) && !_isLoaded)
            {
                auto link = sp_link_create_from_track(_track, 0);
                if (link)
                {
                    auto link_buffer_size = sp_link_as_string(link, nullptr, 0);
                    if (link_buffer_size > 0)
                    {
                        auto s = new char[link_buffer_size + 1];

                        if (sp_link_as_string(link, s, link_buffer_size + 1) > 0)
                            _id = gcnew String(s);

                        delete[] s;
                    }
                    sp_link_release(link);
                }
                _title = SpStringToString(sp_track_name(_track));
                _length = TimeSpan(TimeSpan::TicksPerMillisecond * sp_track_duration(_track));
                _isLoaded = true;
            }
        }
    }
}