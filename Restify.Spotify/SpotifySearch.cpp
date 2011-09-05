
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        SpotifySearchResult::SpotifySearchResult(sp_search *result)
        {
            sp_get_thread_access();

            if (sp_search_error(result) != SP_ERROR_OK)
                return;

            QueryHint = Unstringify(sp_search_did_you_mean(result));

            auto tracks = gcnew List<SpotifyTrack ^>();
            for (int i = 0; i < sp_search_num_tracks(result); i++)
            {
                auto track = sp_search_track(result, i);
                tracks->Add(gcnew SpotifyTrack(track));
            }

            Tracks = tracks;

            Success = true;
        }

        void SpotifySearchResultMessage::Invoke()
        {
            _search->search_complete(_result);
        }
    }
}