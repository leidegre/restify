
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        SpotifyPlaylist::SpotifyPlaylist(sp_playlist *pl)
            : _pl(pl)
            , _list(gcnew ConcurrentDictionary<IntPtr, SpotifyTrack ^>(gcnew IntPtrEqualityComparer()))
        {
            trace("Spotify: (0x%8x) playlist\r\n", pl);
            for (int i = 0; i < sp_playlist_num_tracks(_pl); i++)
            {
                auto track = sp_playlist_track(_pl, i);
                _list->TryAdd(IntPtr(track), gcnew SpotifyTrack(track));
            }
        }

        SpotifyPlaylist::~SpotifyPlaylist()
        {
        }

        void SpotifyPlaylist::Spotify_tracks_added(sp_track * const *tracks, int num_tracks, int position)
        {
            for (int i = 0; i < num_tracks; i++)
            {
                _list->TryAdd(IntPtr(tracks[i]), gcnew SpotifyTrack(tracks[i]));
            }
        }
        
        void SpotifyPlaylist::Spotify_tracks_removed(const int *tracks, int num_tracks)
        {
            for (int i = 0; i < num_tracks; i++)
            {
                SpotifyTrack ^trackObject;
                _list->TryRemove(IntPtr(tracks[i]), trackObject);
            }
        }

        void SpotifyPlaylist::Spotify_playlist_state_changed()
        {
            if (sp_playlist_is_loaded(_pl))
            {
                if (_id == nullptr)
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

                    _title = Unstringify(sp_playlist_name(_pl));
                }
                
                _isLoaded = true;
            }
            
            _is_collaborative = sp_playlist_is_collaborative(_pl);
        }

        void SpotifyPlaylist::Spotify_playlist_metadata_updated()
        {
            for each (KeyValuePair<IntPtr, SpotifyTrack ^> item in _list)
            {
                item.Value->Nudge();
            }
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