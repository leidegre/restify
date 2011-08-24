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
        public RestifyLoginResponse IsLoggedIn(RestifyLogin login)
        {
            return new RestifyLoginResponse { IsLoggedIn = SpotifyManager.Current.IsLoggedIn };
        }

        public RestifyLoginResponse Login(RestifyLogin login)
        {
            if (SpotifyManager.Current.Login(login.UserName, login.Password))
            {
                return new RestifyLoginResponse {
                    IsLoggedIn = true,
                    InstanceName = Program.InstanceName,
                };
            }

            return new RestifyLoginResponse { IsLoggedIn = false };
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
