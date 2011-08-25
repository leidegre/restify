
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        void SpotifyLoginAction::Do(SpotifySession ^session)
        {
            pin_ptr<Byte> userString = &StringToSpString(_user)[0];
            pin_ptr<Byte> passString = &StringToSpString(_pass)[0];
            sp_session_login(session->get_session(), (const char *)userString, (const char *)passString);
        }

        void SpotifyGetPlaylistCollectionAction::Do(SpotifySession ^session)
        {
            auto pl_container = sp_session_playlistcontainer(session->get_session());
            if (pl_container)
            {
                session->_pl_container = gcnew SpotifyPlaylistCollection(session, pl_container);
            }
        }

        void SpotifyPlaylistStateChangeAction::Do(SpotifySession ^session)
        {
            _pl->Spotify_playlist_state_changed();
        }
        
        void SpotifyPlaylistMetadataUpdateAction::Do(SpotifySession ^session)
        {
            _pl->Spotify_playlist_metadata_updated();
        }

        void SpotifyPlayAction::Do(SpotifySession ^session)
        {
            sp_session_player_unload(session->get_session());
            session->_trackToLoad = _track->get_track();
        }

        void SpotifyPlayLinkAction::Do(SpotifySession ^session)
        {
            pin_ptr<Byte> s = &StringToSpString(_trackId)[0];
            sp_link *link = sp_link_create_from_string((const char *)s);
            if (link)
            {
                sp_track *track = sp_link_as_track(link);
                if (track)
                {
                    session->_trackToLoad = track;
                }
                sp_link_release(link);
            }
        }
    }
}