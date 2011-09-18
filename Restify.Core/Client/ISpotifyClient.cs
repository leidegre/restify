using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify.Client
{
    public interface ISpotifyClient : IDisposable
    {
        SpotifyClientLoginError Login(string userName, string password, bool rememberMe);
        ServiceModel.RestifySearchResult Search(string text);
        void Play();
        void PlayPause();
        void Next();
    }
}
