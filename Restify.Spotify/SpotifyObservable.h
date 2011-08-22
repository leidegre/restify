#pragma once

namespace Restify
{
    namespace Client
    {
        public ref class SpotifyObservable : INotifyPropertyChanged
        {
        public:
            virtual event PropertyChangedEventHandler ^PropertyChanged;

        public protected:
            void OnPropertyChanged(String^ propertyName)
            {
                auto e = gcnew PropertyChangedEventArgs(propertyName);
                PropertyChanged(this, e);
            }

        };
    }
}