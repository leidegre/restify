
#include "restify.h"

namespace Restify
{
    namespace Client
    {
        SpotifySearchResult::SpotifySearchResult(sp_search *result)
        {
            if (sp_search_error(result) != SP_ERROR_OK)
                return;

            QueryHint = Unstringify(sp_search_did_you_mean(result));

            auto tracks = gcnew List<SpotifyTrack ^>();
            for (int i = 0; i < sp_search_num_tracks(result); i++)
            {
                auto track = sp_search_track(result, i);

            }

            Success = true;
        }
    }
}