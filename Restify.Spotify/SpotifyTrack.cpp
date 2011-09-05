
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
                _title = Unstringify(sp_track_name(_track));
                StringBuilder ^sb = gcnew StringBuilder();
                for (int i = 0; i < sp_track_num_artists(_track); i++)
                {
                    sp_artist *artist = sp_track_artist(_track, i);
                    if (sp_artist_is_loaded(artist))
                    {
                        if (sb->Length > 0)
                            sb->Append(L", ");
                        
                        sb->Append(Unstringify(sp_artist_name(artist)));
                    }
                }
                _artist = sb->ToString();
                _length = TimeSpan(TimeSpan::TicksPerMillisecond * sp_track_duration(_track));
                _isLoaded = true;
            }
        }
    }
}