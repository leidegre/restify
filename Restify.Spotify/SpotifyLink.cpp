
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        SpotifyLink::SpotifyLink(String ^s)
        {
            if (s == nullptr)
                throw gcnew ArgumentNullException("s");

            sp_get_thread_access();

            pin_ptr<Byte> p = &stringify(s)[0];
            
            sp_link *link = sp_link_create_from_string((const char *)p);
            if (link)
            {
                _type   = (SpotifyLinkType)sp_link_type(link);
                _s      = s;
                
                sp_link_release(link);
            }
        }

        SpotifyTrack ^SpotifyLink::CreateTrack()
        {
            if (_type != SpotifyLinkType::Track)
                throw gcnew InvalidOperationException(L"Invalid link type.");

            SpotifyTrack ^t = nullptr;
            
            pin_ptr<Byte> p = &stringify(_s)[0];

            sp_link *link = sp_link_create_from_string((const char *)p);
            if (link)
            {
                sp_track *track = sp_link_as_track(link);

                // we need to up the ref count on the track 
                // for the track to eventually become `loaded`
                sp_track_add_ref(track); 

                t = gcnew SpotifyTrack(track);

                sp_link_release(link);
            }

            return t;
        }
    }
}