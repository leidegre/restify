
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        SpotifyLink::SpotifyLink(String ^s)
        {
            Contract::Requires(s != nullptr);
            pin_ptr<Byte> p = &Stringify(s)[0];
            sp_link *link = sp_link_create_from_string((const char *)p);
            if (link)
            {
                _link   = link;
                _type   = (SpotifyLinkType)sp_link_type(_link);
                _s      = s;
            }
            else
            {
                _link   = 0;
                _type   = SpotifyLinkType::Invalid;
                _s      = nullptr;
            }
        }

        SpotifyLink::SpotifyLink(sp_link *link)
            : _s(nullptr)
        {
            Contract::Requires(link != nullptr);
            int link_buffer_size = sp_link_as_string(link, nullptr, 0);
            if (link_buffer_size > 0)
            {
                auto s = new char[link_buffer_size + 1];

                if (sp_link_as_string(link, s, link_buffer_size + 1) > 0)
                    _s = Unstringify(s);

                delete[] s;
            }
            _link = link;
            _type = (SpotifyLinkType)sp_link_type(_link);
        }
    }
}