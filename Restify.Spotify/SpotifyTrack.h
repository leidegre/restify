#pragma once

namespace Restify
{
    namespace Client
    {
        public ref class SpotifyTrack
        {
        private:
            sp_track *_track;

        internal:
            SpotifyTrack(sp_track *track)
                : _track(track)
            {
                Nudge();
            }

            sp_track *get_track() { return _track; }

            void Nudge();

        private:
            bool _isLoaded;
        public:
            property bool IsLoaded
            {
                bool get() { return _isLoaded; }
            }

        private:
            String ^_id;
        public:
            property String ^Id
            {
                String ^get() { return _id; }
            }

        private:
            String ^_title;
        public:
            property String ^Title
            {
                String ^get() { return _title; }
            }

        private:
            TimeSpan _length;
        public:
            property TimeSpan Length
            {
                TimeSpan get() { return _length; }
            }
        };
    }
}