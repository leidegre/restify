using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.Text;
using System.Diagnostics;

namespace Restify.Services
{
    public class BackEndService : IBackEndService
    {
        public void Ping()
        {
        }

        public RestifyViewModel GetStatus()
        {
            return new RestifyViewModel { IsLoggedIn = SpotifyManager.Current.IsLoggedIn };
        }

        public RestifyLoginResponse IsLoggedIn(RestifyLogin login)
        {
            return new RestifyLoginResponse { IsLoggedIn = SpotifyManager.Current.IsLoggedIn };
        }

        public RestifyLoginResponse Login(RestifyLogin login)
        {
            if (Debugger.IsAttached)
                Debugger.Break();

            return new RestifyLoginResponse { IsLoggedIn = SpotifyManager.Current.Login(login.UserName, login.Password) };
        }

        public List<RestifyPlaylist> GetPlaylists()
        {
            var playlists = new List<RestifyPlaylist>();

            foreach (var pl in SpotifyManager.Current.GetPlaylists())
            {
                playlists.Add(new RestifyPlaylist {
                    Id = pl.Id,
                    Title = pl.Title,
                    Count = pl.Count,
                });
            }

            return playlists;
        }
    }
}
