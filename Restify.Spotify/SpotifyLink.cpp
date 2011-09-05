
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

            pin_ptr<Byte> p = &Stringify(s)[0];
            
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

            SpotifyTrack ^track = nullptr;
            
            pin_ptr<Byte> p = &Stringify(_s)[0];

            sp_link *link = sp_link_create_from_string((const char *)p);
            if (link)
            {
                track = gcnew SpotifyTrack(sp_link_as_track(link));
                sp_link_release(link);
            }

            return track;
        }
    }
}