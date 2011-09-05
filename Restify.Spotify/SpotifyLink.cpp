
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        void SpotifyLink::Initialize(String ^s)
        {
            if (s == nullptr)
                throw gcnew ArgumentNullException("s");

            pin_ptr<Byte> p = &Stringify(s)[0];
            
            sp_link *link = sp_link_create_from_string((const char *)p);
            if (link)
            {
                _type   = (SpotifyLinkType)sp_link_type(link);
                _s      = s;
                
                sp_link_release(link);
            }
        }
    }
}