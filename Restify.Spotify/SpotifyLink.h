

namespace Restify
{
    namespace Client
    {
        public enum class SpotifyLinkType 
        {
            Invalid = SP_LINKTYPE_INVALID, 
            Track = SP_LINKTYPE_TRACK, 
            Album = SP_LINKTYPE_ALBUM, 
            Artist = SP_LINKTYPE_ARTIST, 
            Search = SP_LINKTYPE_SEARCH, 
            Playlist = SP_LINKTYPE_PLAYLIST, 
            Profile = SP_LINKTYPE_PROFILE, 
            Starred = SP_LINKTYPE_STARRED, 
            LocalTrack = SP_LINKTYPE_LOCALTRACK, 
            Image = SP_LINKTYPE_IMAGE 
        };

        public value struct SpotifyLink
        {
        private:
            sp_link *_link;
            SpotifyLinkType _type;
            String ^_s;
            
        internal:
            sp_link *get_link() { return _link; }

            SpotifyLink(sp_link *link);

        public:
            property SpotifyLinkType Type { SpotifyLinkType get() { return _type; } }

            property bool HasValue { bool get() { return _s != nullptr; } }
            property String ^Value { String ^get() { return _s; } }

            SpotifyLink(String ^s);
        };
    }
}