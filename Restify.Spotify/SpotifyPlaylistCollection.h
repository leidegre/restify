#pragma once

namespace Restify
{
    namespace Client
    {
        public delegate void SpotifyPlaylistCollectionLoaded(SpotifyPlaylistCollection ^sender);

        public ref class SpotifyPlaylistCollection
        {
        private:
            SpotifySession ^_session;
            sp_playlistcontainer *_pl_container;
            sp_playlistcontainer_callbacks *_pl_container_callbacks;
            sp_playlist_callbacks *_pl_callbacks;

            gcroot<SpotifyPlaylistCollection ^> *_gcroot;

            ConcurrentDictionary<IntPtr, SpotifyPlaylist ^> ^_list;

        internal:
            SpotifyPlaylistCollection(sp_playlistcontainer *pl_container);
            ~SpotifyPlaylistCollection();

            void sp_playlistcontainer_container_loaded();

            void sp_playlistcontainer_playlist_added(sp_playlist *pl, int position);
            void sp_playlistcontainer_playlist_removed(sp_playlist *pl, int position);
            void sp_playlistcontainer_playlist_moved(sp_playlist *pl);
            
            void playlist_metadata_updated(sp_playlist *pl);

            void Spotify_tracks_added(sp_playlist *pl, sp_track * const *tracks, int num_tracks, int position);
            void Spotify_tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks);

            sp_playlistcontainer *get_pl_container() { return _pl_container; }

        public:
            event Action ^Loaded;
            event Action<SpotifyPlaylist ^> ^Added;
            event Action<SpotifyPlaylist ^> ^Removed;

        private:
            bool _isLoaded;

        public:
            property bool IsLoaded { bool get() { return _isLoaded; } }

            property int Count
            {
                int get() { return _list->Count; }
            }

            List<SpotifyPlaylist ^> ^ToList()
            {
                auto list = gcnew List<SpotifyPlaylist ^>();
                for each (KeyValuePair<IntPtr, SpotifyPlaylist ^> item in _list)
                {
                    list->Add(item.Value);
                }
                return list;
            }
        };
    }
}