
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        void SpotifyTrack::Update()
        {
            if (sp_track_is_loaded(_track))
            {
                sp_link *track_link = sp_link_create_from_track(_track, 0);
                if (track_link)
                {
                    auto link_buffer_size = sp_link_as_string(track_link, nullptr, 0);
                    if (link_buffer_size > 0)
                    {
                        auto s = new char[link_buffer_size + 1];

                        if (sp_link_as_string(track_link, s, link_buffer_size + 1) > 0)
                            _id = unstringify(s);

                        delete[] s;
                    }
                    sp_link_release(track_link);
                }

                _title = unstringify(sp_track_name(_track));

                auto sb = gcnew StringBuilder();
                for (int i = 0; i < sp_track_num_artists(_track); i++)
                {
                    sp_artist *artist = sp_track_artist(_track, i);
                    if (sp_artist_is_loaded(artist))
                    {
                        if (sb->Length > 0)
                            sb->Append(L", ");
                        
                        sb->Append(unstringify(sp_artist_name(artist)));
                    }
                }

                _artist = sb->ToString();

                _length = TimeSpan(TimeSpan::TicksPerMillisecond * sp_track_duration(_track));

                _isLoaded = true;
            }
            _error = (SpotifyError)sp_track_error(_track);
        }
    }
}

