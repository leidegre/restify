
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
            if (sp_playlist_is_loaded(_pl->get_playlist()))
            {
                _pl->Load();
            }
        }
    }
}