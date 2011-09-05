#pragma once

namespace Restify
{
    namespace Client
    {
        public delegate void SpotifyPlaylistTracksEventHandler(SpotifyPlaylist^ pl, IList<SpotifyTrack ^> ^tracks);

        public ref class SpotifyPlaylist
        {
        private:
            sp_playlist *_pl;
            
            gcroot<SpotifyPlaylist ^>* _userdata;

            bool _isLoaded;
            
            ConcurrentDictionary<IntPtr, SpotifyTrack ^> ^_list;
            
            String ^_id;
            String ^_title;
            bool _is_collaborative;

        internal:
             SpotifyPlaylist(sp_playlist *pl);
             ~SpotifyPlaylist();

             sp_playlist *get_playlist() { return _pl; }
             
             void Spotify_playlist_state_changed();
             void Spotify_playlist_metadata_updated();
             void Spotify_tracks_added(sp_track * const *tracks, int num_tracks, int position);
             void Spotify_tracks_removed(const int *tracks, int num_tracks);

        public:
            property bool IsLoaded 
            { 
                bool get() { return _isLoaded; } 
            }

            property bool IsCollaborative
            { 
                bool get() { return _is_collaborative; }
            }

            property int Count
            {
                int get() { return _list->Count; }
            }

            property String ^Id
            { 
                String ^get() 
                { 
                    return _id;
                } 
            }

            property String ^Title
            {
                String ^get()
                {
                    return _title;
                }
            }

            List<SpotifyTrack ^> ^ToList();
        };
    }
}