
#include "restify.h"

namespace Restify
{
    namespace Spotify
    {
        inline gcroot<SpotifyPlaylist ^> GetPlaylist(void *userdata)
        {
            return *static_cast<gcroot<SpotifyPlaylist ^> *>(userdata);
        }

        void SP_CALLCONV Spotify_tracks_added(sp_playlist *pl, sp_track * const *tracks, int num_tracks, int position, void *userdata)
        {
            trace("Spotify_tracks_added\r\n");
            GetPlaylist(userdata)->OnTracksAdded(tracks, num_tracks, position);
        }

        void SP_CALLCONV Spotify_tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks, void *userdata)
        {
            trace("Spotify_tracks_removed\r\n");
            GetPlaylist(userdata)->OnTracksRemoved(tracks, num_tracks);
        }

        void SP_CALLCONV Spotify_tracks_moved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *userdata)
        {
            trace("Spotify_tracks_moved\r\n");
            GetPlaylist(userdata)->OnTracksMoved(tracks, num_tracks, new_position);
        }

        void SP_CALLCONV Spotify_playlist_renamed(sp_playlist *pl, void *userdata)
        {
            trace("Spotify_playlist_renamed\r\n");
            GetPlaylist(userdata)->OnRenamed();
        }

        SpotifyPlaylist::SpotifyPlaylist(sp_playlist *pl)
            : _pl(pl)
            , _pl_callbacks(new sp_playlist_callbacks)
        {
            _userdata = new gcroot<SpotifyPlaylist ^>(this);

            _list = gcnew List<SpotifyTrack ^>();

            RtlZeroMemory(_pl_callbacks, sizeof(sp_playlist_callbacks));
            _pl_callbacks->tracks_added = &Spotify_tracks_added;
            _pl_callbacks->tracks_removed = &Spotify_tracks_removed;
            _pl_callbacks->tracks_moved  = &Spotify_tracks_moved;
            _pl_callbacks->playlist_renamed = &Spotify_playlist_renamed;
            sp_playlist_add_callbacks(_pl, _pl_callbacks, _userdata);
        }

        SpotifyPlaylist::~SpotifyPlaylist()
        {
            sp_playlist_remove_callbacks(_pl, _pl_callbacks, _userdata);
            delete _userdata;
            delete _pl_callbacks;
        }

        void SpotifyPlaylist::OnTracksAdded(sp_track * const *tracks, int num_tracks, int position)
        {
            for (int i = 0; i < num_tracks; i++)
            {
                _list->Insert(position + i, gcnew SpotifyTrack(tracks[i]));
            }
        }

        void SpotifyPlaylist::OnTracksRemoved(const int *tracks, int num_tracks)
        {
            for (int i = 0; i < num_tracks; i++)
            {
                // mark for delete
                _list[tracks[i]] = nullptr;
            }
            // clean up
            for (int i = 0; i < _list->Count; i++)
            {
                if (_list[i] == nullptr)
                {
                    _list->RemoveAt(i--);
                }
            }
        }

        void SpotifyPlaylist::OnTracksMoved(const int *tracks, int num_tracks, int new_position)
        {
            auto move_list = gcnew List<SpotifyTrack ^>();
            for (int i = 0; i < num_tracks; i++)
            {
                move_list->Add(_list[tracks[i]]);
                // mark for delete
                _list[tracks[i]] = nullptr;
            }
            for (int i = 0; i < num_tracks; i++)
            {
                _list->Insert(new_position + i, move_list[i]);
            }
            // clean up
            for (int i = 0; i < _list->Count; i++)
            {
                if (_list[i] == nullptr)
                {
                    _list->RemoveAt(i--);
                }
            }
        }

        void SpotifyPlaylist::OnRenamed()
        {
        }
    }
}