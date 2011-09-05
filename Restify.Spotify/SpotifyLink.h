

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

        [System::Diagnostics::DebuggerDisplay("{_s, nq}, Type = {_type}")]
        public value struct SpotifyLink : IEquatable<SpotifyLink>
        {
        private:
            SpotifyLinkType _type;
            String ^_s;
            
        internal:
            void Initialize(String ^s);

        public:
            property SpotifyLinkType Type { SpotifyLinkType get() { return _type; } }
            
            property bool HasValue { bool get() { return _s != nullptr; } }
            property String ^Value { String ^get() { return _s; } }

            virtual bool Equals(Object ^obj) override
            {
                auto value = safe_cast<Nullable<SpotifyLink>>(obj);
                if (value.HasValue)
                {
                    return Equals(value.Value);
                }
                return false;
            }

            virtual bool Equals(SpotifyLink link)
            {
                return _s == link._s;
            }

            virtual int GetHashCode() override
            {
                return _s != nullptr ? _s->GetHashCode() : 0;
            }

            virtual String ^ToString() override
            {
                return _s;
            }
        };
    }
}