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
            SpotifyPlaylistCollection(SpotifySession ^session, sp_playlistcontainer *pl_container);
            ~SpotifyPlaylistCollection();

            void container_loaded();
            void playlist_added(sp_playlist *pl, int position);
            void playlist_removed(sp_playlist *pl, int position);
            void playlist_state_changed(sp_playlist *pl);

            void tracks_added(sp_playlist *pl, sp_track * const *tracks, int num_tracks, int position);
            void tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks);

            sp_playlistcontainer *get_pl_container() { return _pl_container; }

        public:
            property bool IsLoaded
            {
                bool get()
                {
                    return sp_playlistcontainer_is_loaded(_pl_container);
                }
            }

            property int Count
            {
                int get()
                {
                    return _list->Count;
                }
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