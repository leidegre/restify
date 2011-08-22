#pragma once

namespace Restify
{
    namespace Client
    {
        public delegate void SpotifyPlaylistTracksEventHandler(SpotifyPlaylist^ pl, IList<SpotifyTrack ^> ^tracks);

        public ref class SpotifyPlaylist : SpotifyObservable
        {
        private:
            sp_playlist *_pl;
            
            gcroot<SpotifyPlaylist ^>* _userdata;

            bool _isLoaded;
            
            ConcurrentDictionary<IntPtr, SpotifyTrack ^> ^_list;
            
            String ^_id;
            String ^_title;

        internal:
             SpotifyPlaylist(sp_playlist *pl);
             ~SpotifyPlaylist();

             sp_playlist *get_playlist() { return _pl; }
             
             void Load();

             void Add(sp_track *track);
             void Remove(sp_track *track);

        public:
            property bool IsLoaded 
            { 
                bool get() { return _isLoaded; } 
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