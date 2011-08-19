#pragma once

namespace Restify
{
    namespace Spotify
    {
        public delegate void SpotifyPlaylistTracksEventHandler(SpotifyPlaylist^ pl, IList<SpotifyTrack ^> ^tracks);

        public ref class SpotifyPlaylist : SpotifyObservable
        {
        private:
            sp_playlist *_pl;
            sp_playlist_callbacks *_pl_callbacks;
            gcroot<SpotifyPlaylist ^>* _userdata;

            String ^_id;

            List<SpotifyTrack ^>^ _list;

        internal:
             SpotifyPlaylist(sp_playlist *pl);
             ~SpotifyPlaylist();

             sp_playlist *get_playlist() { return _pl; }

             void OnTracksAdded(sp_track * const *tracks, int num_tracks, int position);
             void OnTracksRemoved(const int *tracks, int num_tracks);
             void OnTracksMoved(const int *tracks, int num_tracks, int new_position);
             void OnRenamed();
             
        public:
            property String ^Id
            { 
                String ^get() 
                { 
                    if (_id == nullptr)
                    {
                        if (sp_playlist_is_loaded(_pl))
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
                        }
                    }
                    return _id;
                } 
            }

            property bool IsLoaded 
            { 
                bool get() { return sp_playlist_is_loaded(_pl); } 
            }

            property int Count
            {
                int get() { return sp_playlist_num_tracks(_pl); }
            }

            property String ^Title
            {
                String ^get()
                {
                    auto s = sp_playlist_name(_pl);
                    return gcnew String(s);
                }
            }
        };
    }
}