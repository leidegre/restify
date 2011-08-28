
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        gcroot<SpotifyPlaylistCollection ^> GetSpotifyPlaylistCollection(void *userdata)
        {
            return *static_cast<gcroot<SpotifyPlaylistCollection ^> *>(userdata);
        }

        // container callbacks
        
        void SP_CALLCONV Spotify_container_loaded(sp_playlistcontainer *pl_container, void *userdata)
        {
            trace("Spotify_container_loaded\r\n");
            GetSpotifyPlaylistCollection(userdata)->container_loaded();
        }
        
        void SP_CALLCONV Spotify_playlist_added(sp_playlistcontainer *pc, sp_playlist *pl, int position, void *userdata)
        {
            trace("Spotify_playlist_added\r\n");
            GetSpotifyPlaylistCollection(userdata)->playlist_added(pl, position);
        }
        
        void SP_CALLCONV Spotify_playlist_removed(sp_playlistcontainer *pc, sp_playlist *pl, int position, void *userdata)
        {
            trace("Spotify_playlist_removed\r\n");
            GetSpotifyPlaylistCollection(userdata)->playlist_removed(pl, position);
        }

        void SP_CALLCONV Spotify_playlist_moved(sp_playlistcontainer *pc, sp_playlist *pl, int position, int new_position, void *userdata)
        {
            trace("Spotify_playlist_moved\r\n");
        }

        // <sp_playlist_callbacks>

        void SP_CALLCONV sp_playlist_tracks_added(sp_playlist *pl, sp_track * const *tracks, int num_tracks, int position, void *userdata)
        {
            trace("sp_playlist::tracks_added\r\n");
            GetSpotifyPlaylistCollection(userdata)->Spotify_tracks_added(pl, tracks, num_tracks, position);
        }

        void SP_CALLCONV sp_playlist_tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks, void *userdata)
        {
            trace("sp_playlist::tracks_removed\r\n");
            GetSpotifyPlaylistCollection(userdata)->Spotify_tracks_removed(pl, tracks, num_tracks);
        }

        void SP_CALLCONV sp_playlist_tracks_moved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *userdata)
        {
            trace("sp_playlist::tracks_moved\r\n");
        }

        void SP_CALLCONV sp_playlist_playlist_renamed(sp_playlist *pl, void *userdata)
        {
            trace("sp_playlist::playlist_renamed\r\n");
        }
        
        void SP_CALLCONV sp_playlist_playlist_state_changed(sp_playlist *pl, void *userdata)
        {
            trace("sp_playlist::playlist_state_changed\r\n");
            GetSpotifyPlaylistCollection(userdata)->playlist_state_changed(pl);
        }

        void SP_CALLCONV sp_playlist_playlist_metadata_updated(sp_playlist *pl, void *userdata)
        {
            trace("sp_playlist::playlist_metadata_updated\r\n");
            GetSpotifyPlaylistCollection(userdata)->playlist_metadata_updated(pl);
        }
        
        // </sp_playlist_callbacks>

        SpotifyPlaylistCollection::SpotifyPlaylistCollection(SpotifySession ^session, sp_playlistcontainer *pl_container)
            : _session(session)
            , _pl_container(pl_container)
            , _pl_container_callbacks(new sp_playlistcontainer_callbacks)
            , _pl_callbacks(new sp_playlist_callbacks)
            , _list(gcnew ConcurrentDictionary<IntPtr, SpotifyPlaylist ^>(gcnew IntPtrEqualityComparer()))
        {
            _gcroot = new gcroot<SpotifyPlaylistCollection ^>(this);
            
            RtlZeroMemory(_pl_container_callbacks, sizeof(sp_playlistcontainer_callbacks));
            _pl_container_callbacks->playlist_added = &Spotify_playlist_added;
            _pl_container_callbacks->playlist_removed = &Spotify_playlist_removed;
            _pl_container_callbacks->playlist_moved = &Spotify_playlist_moved;
            _pl_container_callbacks->container_loaded = &Spotify_container_loaded;
            sp_playlistcontainer_add_callbacks(_pl_container, _pl_container_callbacks, _gcroot);

            RtlZeroMemory(_pl_callbacks, sizeof(sp_playlist_callbacks));
            _pl_callbacks->tracks_added = &sp_playlist_tracks_added;
            _pl_callbacks->tracks_removed = &sp_playlist_tracks_removed;
            _pl_callbacks->tracks_moved  = &sp_playlist_tracks_moved;
            _pl_callbacks->playlist_renamed = &sp_playlist_playlist_renamed;
            _pl_callbacks->playlist_state_changed = &sp_playlist_playlist_state_changed;
            _pl_callbacks->playlist_metadata_updated  = &sp_playlist_playlist_metadata_updated;

            for (int i = 0; i < sp_playlistcontainer_num_playlists(_pl_container); i++)
            {
                auto pl = sp_playlistcontainer_playlist(_pl_container, i);
                sp_playlist_add_callbacks(pl, _pl_callbacks, _gcroot);
                _list->TryAdd(IntPtr(pl), gcnew SpotifyPlaylist(pl));
            }
        }
        
        SpotifyPlaylistCollection::~SpotifyPlaylistCollection()
        {
            delete _pl_callbacks;
            delete _pl_container_callbacks;
            delete _gcroot;
        }

        void SpotifyPlaylistCollection::container_loaded()
        {
            //auto handle = _session->_getPlaylistCollection;
            //if (handle != nullptr)
            //    handle->Set();
        }

        void SpotifyPlaylistCollection::playlist_added(sp_playlist *pl, int position)
        {
            sp_playlist_add_callbacks(pl, _pl_callbacks, _gcroot);
            _list->TryAdd(IntPtr(pl), gcnew SpotifyPlaylist(pl));
        }

        void SpotifyPlaylistCollection::playlist_removed(sp_playlist *pl, int position)
        {
            SpotifyPlaylist ^plObject;
            _list->TryRemove(IntPtr(pl), plObject);
        }

        void SpotifyPlaylistCollection::playlist_state_changed(sp_playlist *pl)
        {
            //SpotifyPlaylist ^plObject;
            //if (_list->TryGetValue(IntPtr(pl), plObject))
            //{
            //    _session->Do(gcnew SpotifyPlaylistStateChangeAction(plObject));
            //}
        }

        void SpotifyPlaylistCollection::playlist_metadata_updated(sp_playlist *pl)
        {
            //SpotifyPlaylist ^plObject;
            //if (_list->TryGetValue(IntPtr(pl), plObject))
            //{
            //    _session->Do(gcnew SpotifyPlaylistMetadataUpdateAction(plObject));
            //}
        }

        void SpotifyPlaylistCollection::Spotify_tracks_added(sp_playlist *pl, sp_track * const *tracks, int num_tracks, int position)
        {
            trace("Spotify: (0x%8x) tracks_added (%i)\r\n", pl, num_tracks);
            SpotifyPlaylist ^plObject;
            if (_list->TryGetValue(IntPtr(pl), plObject))
            {
                plObject->Spotify_tracks_added(tracks, num_tracks, position);
            }
        }

        void SpotifyPlaylistCollection::Spotify_tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks)
        {
            SpotifyPlaylist ^plObject;
            if (_list->TryGetValue(IntPtr(pl), plObject))
            {
                plObject->Spotify_tracks_removed(tracks, num_tracks);
            }
        }
    }
}