using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Restify.ServiceModel.Composition;
using Restify.Client;
using System.ComponentModel.Composition;

namespace Restify.ServiceModel
{
    [ComposableServiceExport("/restify/user/{0}/auth", typeof(LoginService), AddressMetadata = new[] { "RESTify.AgentId" })]
    class LoginService : ILoginService
    {
        private ISpotifyClient spotifyClient;

        [ImportingConstructor]
        public LoginService(ISpotifyClient spotifyClient)
        {
            this.spotifyClient = spotifyClient;
        }

        public LoginResponse Login(LoginRequest login)
        {
            return new LoginResponse{
                LoginError = this.spotifyClient.Login(login.UserName, login.Password, login.RememberMe),
            };
        }

        void IDisposable.Dispose()
        {
        }
    }
}
