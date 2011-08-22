
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        SpotifyPlaylist::SpotifyPlaylist(sp_playlist *pl)
            : _pl(pl)
            , _list(gcnew ConcurrentDictionary<IntPtr, SpotifyTrack ^>())
        {
        }

        SpotifyPlaylist::~SpotifyPlaylist()
        {
        }

        void SpotifyPlaylist::Add(sp_track *track)
        {
            _list->TryAdd(IntPtr(track), gcnew SpotifyTrack(track));
        }
        
        void SpotifyPlaylist::Remove(sp_track *track)
        {
            SpotifyTrack ^trackObject;
            _list->TryRemove(IntPtr(track), trackObject);
        }

        void SpotifyPlaylist::Load()
        {
            auto link = sp_link_create_from_playlist(_pl);
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
            
            _title = SpStringToString(sp_playlist_name(_pl));

            _isLoaded = true;
        }

        List<SpotifyTrack ^> ^SpotifyPlaylist::ToList()
        {
            auto list = gcnew List<SpotifyTrack ^>();
            for each (KeyValuePair<IntPtr, SpotifyTrack ^> item in _list)
            {
                list->Add(item.Value);
            }
            return list;
        }
    }
}