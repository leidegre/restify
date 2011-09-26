using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Restify.ServiceModel;

namespace Restify.Client
{
    public interface ISpotifyClient : IDisposable
    {
        SpotifyClientLoginError Login(string userName, string password, bool rememberMe);
        RestifySearchResult Search(string text);
        void Play();
        void PlayPause();
        void Next();
    }
}
