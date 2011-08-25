
namespace Restify
{
    namespace Client
    {
        // these are all private classes for
        // synchronizing calls to the API

        // all calls to libspotify are serializing
        // through these objects
        // (this is important because libspotify is not thread-safe)

        ref class ISpotifyAction abstract
        {
        private:
            ManualResetEventSlim ^_signal;

        public:
            ISpotifyAction()
                : _signal(gcnew ManualResetEventSlim(false))
            {
            }

            ~ISpotifyAction()
            {
                delete _signal;
            }

            bool Wait() 
            { 
                return _signal->Wait(30 * 1000); 
            }

            void Set() 
            {
                _signal->Set();
            }

            virtual void Do(SpotifySession ^session) = 0;
        };

        ref class SpotifyLoginAction : ISpotifyAction
        {
        private:
            String ^_user;
            String ^_pass;

        public:
            SpotifyLoginAction(String ^user, String ^pass)
                : _user(user)
                , _pass(pass)
            {
            }

            virtual void Do(SpotifySession ^session) override;
        };

        ref class SpotifyGetPlaylistCollectionAction : ISpotifyAction
        {
        private:
        public:
            virtual void Do(SpotifySession ^session) override;
        };

        ref class SpotifyPlaylistStateChangeAction : ISpotifyAction
        {
        private:
            SpotifyPlaylist ^_pl;

        public:
            SpotifyPlaylistStateChangeAction(SpotifyPlaylist ^pl)
                : _pl(pl)
            {
            }

            virtual void Do(SpotifySession ^session) override;
        };

        ref class SpotifyPlaylistMetadataUpdateAction : ISpotifyAction
        {
        private:
            SpotifyPlaylist ^_pl;

        public:
            SpotifyPlaylistMetadataUpdateAction(SpotifyPlaylist ^pl)
                : _pl(pl)
            {
            }

            virtual void Do(SpotifySession ^session) override;
        };

        ref class SpotifyPlayAction : ISpotifyAction
        {
        private:
            SpotifyTrack ^_track;

        public:
            SpotifyPlayAction(SpotifyTrack ^track)
                : _track(track)
            {
            }

            virtual void Do(SpotifySession ^session) override;
        };

        ref class SpotifyPlayLinkAction : ISpotifyAction
        {
        private:
            String ^_trackId;

        public:
            SpotifyPlayLinkAction(String ^trackId)
                : _trackId(trackId)
            {
            }

            virtual void Do(SpotifySession ^session) override;
        };
    }
}