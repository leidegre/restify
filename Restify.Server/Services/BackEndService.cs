using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.Text;

namespace Restify.Services
{
    public class BackEndService : IBackEndService
    {
        public RestifyViewModel GetStatus()
        {
            return new RestifyViewModel { IsLoggedIn = SpotifyManager.Current.IsLoggedIn };
        }

        public bool Login(RestifyLogin login)
        {
            return SpotifyManager.Current.Login(login.UserName, login.Password);
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
