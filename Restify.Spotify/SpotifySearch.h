#pragma once

namespace Restify
{
    namespace Client
    {
        public ref class SpotifySearchResult
        {
        internal:
            SpotifySearchResult(sp_search *result);

        public:
            property bool Success;
            property String ^QueryHint;
            property List<SpotifyTrack ^> ^Tracks;
            property int TracksTotal;
            property List<SpotifyAlbum ^> ^Albums;
            property int AlbumsTotal;
            property List<SpotifyArtist ^> ^Artists;
            property int ArtistTotal;
        };
        
        public ref class SpotifySearchQuery
        {
        internal:
            void search_complete(sp_search *result)
            {
                Completed(gcnew SpotifySearchResult(result));
            }

        public:
            event Action<SpotifySearchResult ^> ^Completed;
            
            property String ^Query;
            property int TrackOffset;
            property int TrackCount;
            property int AlbumOffset;
            property int AlbumCount;
            property int ArtistOffset;
            property int ArtistCount;
        };

        ref class SpotifySearchResultMessage : ISpotifyMessage
        {
        private:
            SpotifySearchQuery ^_search; 
            sp_search *_result;

        public:
            SpotifySearchResultMessage(SpotifySearchQuery ^search, sp_search *result)
                : _search(search)
                , _result(result)
            {
            }
            
            ~SpotifySearchResultMessage()
            {
                if (_result)
                {
                    sp_search_release(_result);
                    _result = nullptr;
                }
            }

            virtual void Invoke();
        };
    }
}
