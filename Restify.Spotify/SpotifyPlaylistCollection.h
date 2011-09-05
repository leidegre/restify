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

            sp_playlistcontainer *get_pl_container() { return _pl_container; }
            
            // playlist container callbacks
            void container_loaded();
            void playlist_added(sp_playlist *pl, int position);
            void playlist_removed(sp_playlist *pl, int position);
            
            // playlist callbacks
            void tracks_added(sp_playlist *pl, sp_track * const *tracks, int num_tracks, int position);
            void tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks);
            void playlist_renamed(sp_playlist *pl);
            void playlist_state_changed(sp_playlist *pl);
            void playlist_metadata_updated(sp_playlist *pl);

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