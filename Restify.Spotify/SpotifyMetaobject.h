#pragma once

#include "restify.h"

namespace Restify
{
    namespace Client
    {
        public interface class ISpotifyMetaobject
        {
            property bool IsLoaded { bool get(); };
            property SpotifyError Error { SpotifyError get(); }
            void Update();
        };

        public ref class SpotifyTrack : ISpotifyMetaobject
        {
        private:
            sp_track *_track;
            bool _isLoaded;
            SpotifyError _error;
            String ^_id;
            String ^_title;
            String ^_artist;
            TimeSpan _length;

        internal:
            sp_track *get_track() { return _track; }

            SpotifyTrack(sp_track* track)
                : _track(track)
            {
                Update();
            }

        public:
            virtual property bool IsLoaded { bool get() { return _isLoaded; } }
            virtual property SpotifyError Error { SpotifyError get() { return _error; } }
            virtual void Update();

            property String ^Id { String ^get() { return _id; } };
            property String ^Title { String ^get() { return _title; } };
            property String ^Artist { String ^get(){ return _artist; } };
            property TimeSpan Length { TimeSpan get() { return _length; } };
        };
    }
}