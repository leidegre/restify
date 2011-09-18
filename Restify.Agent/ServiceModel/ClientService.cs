using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.Text;
using System.Diagnostics;
using Restify.Client;
using Restify.ServiceModel.Composition;
using System.ComponentModel.Composition;

namespace Restify.ServiceModel
{
    [ComposableServiceExport("/restify/user/{0}", typeof(ClientService), AddressMetadata = new[] { ConfigurationExports.AgentId })]
    class ClientService : IClientService
    {
        private ISpotifyClient spotifyClient;

        [ImportingConstructor]
        public ClientService(ISpotifyClient spotifyClient)
        {
            this.spotifyClient = spotifyClient;
        }

        public RestifySearchResult Search(string text)
        {
            return spotifyClient.Search(text);
        }

        public void Play()
        {
            spotifyClient.Play();
        }

        public void PlayPause()
        {
            spotifyClient.PlayPause();
        }

        public void Next()
        {
            spotifyClient.Next();
        }

        void IDisposable.Dispose()
        {
        }
    }
}
