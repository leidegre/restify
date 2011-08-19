#pragma once

namespace Restify
{
    namespace Spotify
    {
        public delegate void SpotifyPlaylistCollectionLoaded(SpotifyPlaylistCollection ^sender);

        public ref class SpotifyPlaylistCollection
        {
        private:
            SpotifySession ^_session;
            sp_playlistcontainer *_pl_container;
            sp_playlistcontainer_callbacks *_pl_container_callbacks;
            gcroot<SpotifyPlaylistCollection ^> *_gcroot;

            List<SpotifyPlaylist ^> ^_list;

        internal:
            SpotifyPlaylistCollection(SpotifySession ^session, sp_playlistcontainer *pl_container);
            ~SpotifyPlaylistCollection();

            sp_playlistcontainer *get_pl_container() { return _pl_container; }

            void OnLoaded();
            void OnAdded(sp_playlist *playlist, int position);
            void OnRemoved(sp_playlist *playlist, int position);
            void OnMoved(sp_playlist *playlist, int position, int new_position);

        public:
            event SpotifyPlaylistCollectionLoaded^ Loaded;

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

            property SpotifyPlaylist ^default[int]
            {
                SpotifyPlaylist ^get(int index)
                {
                    return _list[index];
                }
            }
        };
    }
}