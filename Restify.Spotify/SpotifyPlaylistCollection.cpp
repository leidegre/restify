
#include "restify.h"

namespace Restify
{
    namespace Spotify
    {
        void Spotify_playlist_added(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata)
        {
            trace("Spotify_playlist_added\r\n");
            auto pl_container1 = *static_cast<gcroot<SpotifyPlaylistCollection ^> *>(userdata);
            pl_container1->OnAdded(playlist, position);
        }
        
        void Spotify_playlist_removed(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata)
        {
            trace("Spotify_playlist_removed\r\n");
            auto pl_container1 = *static_cast<gcroot<SpotifyPlaylistCollection ^> *>(userdata);
            pl_container1->OnRemoved(playlist, position);
        }

        void Spotify_playlist_moved(sp_playlistcontainer *pc, sp_playlist *playlist, int position, int new_position, void *userdata)
        {
            trace("Spotify_playlist_moved\r\n");
            auto pl_container1 = *static_cast<gcroot<SpotifyPlaylistCollection ^> *>(userdata);
            pl_container1->OnMoved(playlist, position, new_position);
        }

        void SP_CALLCONV Spotify_container_loaded(sp_playlistcontainer *pl_container, void *userdata)
        {
            trace("Spotify_container_loaded\r\n");
            auto pl_container1 = *static_cast<gcroot<SpotifyPlaylistCollection ^> *>(userdata);
            pl_container1->OnLoaded();
        }

        SpotifyPlaylistCollection::SpotifyPlaylistCollection(SpotifySession ^session, sp_playlistcontainer *pl_container)
            : _session(session)
            , _pl_container(pl_container)
            , _pl_container_callbacks(new sp_playlistcontainer_callbacks)
        {
            _gcroot = new gcroot<SpotifyPlaylistCollection ^>(this);

            _list = gcnew List<SpotifyPlaylist ^>();

            int count = sp_playlistcontainer_num_playlists(_pl_container);
            for (int i = 0; i < count; i++)
            {
                _list->Add(gcnew SpotifyPlaylist(sp_playlistcontainer_playlist(_pl_container, i)));
            }

            RtlZeroMemory(_pl_container_callbacks, sizeof(sp_playlistcontainer_callbacks));
            _pl_container_callbacks->container_loaded = &Spotify_container_loaded;
            sp_playlistcontainer_add_callbacks(_pl_container, _pl_container_callbacks, _gcroot);
        }
        
        SpotifyPlaylistCollection::~SpotifyPlaylistCollection()
        {
            delete _pl_container_callbacks;
            delete _gcroot;
        }

        void SpotifyPlaylistCollection::OnLoaded()
        {
            Loaded(this);
        }

        void SpotifyPlaylistCollection::OnAdded(sp_playlist *playlist, int position)
        {
            LOCK(_list);
            _list->Add(gcnew SpotifyPlaylist(playlist));
        }

        void SpotifyPlaylistCollection::OnRemoved(sp_playlist *playlist, int position)
        {
            LOCK(_list);
            for (int i = 0; i < _list->Count; i++)
            {
                if (_list[i]->get_playlist() == playlist)
                {
                    _list->RemoveAt(i);
                    break;
                }
            }
        }

        void SpotifyPlaylistCollection::OnMoved(sp_playlist *playlist, int position, int new_position)
        {
        }
    }
}