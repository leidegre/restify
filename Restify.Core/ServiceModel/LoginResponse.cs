using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Restify.Client;

namespace Restify.ServiceModel
{
    public class LoginResponse
    {
        public bool IsLoggedIn { get; set; }
        public SpotifyClientLoginError LoginError { get; set; }
        public string InstanceName { get; set; }
    }
}
