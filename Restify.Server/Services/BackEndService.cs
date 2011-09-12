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
        public RestifyLoginResponse IsLoggedIn(RestifyLoginRequest login)
        {
            return new RestifyLoginResponse { IsLoggedIn = SpotifyManager.Current.IsLoggedIn };
        }

        public RestifyLoginResponse Login(RestifyLoginRequest login)
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

            //foreach (var pl in SpotifyManager.Current.GetPlaylists())
            //{
            //    playlists.Add(new RestifyPlaylist {
            //        Id = pl.Id,
            //        Title = pl.Title,
            //        Count = pl.Count,
            //    });
            //}

            return playlists;
        }

        public List<RestifyTrack> GetPlaylist(RestifyPlaylistRequest playlist)
        {
            var list = new List<RestifyTrack>();
            
            //var pl = SpotifyManager.Current.GetPlaylist(playlist.Id);

            //foreach (var track in pl.ToList())
            //{
            //    list.Add(new RestifyTrack {
            //        Id = track.Id,
            //        Title = track.Title,
            //        Length = RestifyTrack.ToString(track.Length),
            //    });
            //}

            return list;
        }

        public void Play()
        {
            SpotifyManager.Current.Play();
        }

        public void PlayPause()
        {
            SpotifyManager.Current.PlayPause();
        }

        public void Next()
        {
            SpotifyManager.Current.Next();
        }

        public void Enqueue(string trackId)
        {
        }

        public RestifyTrack Dequeue()
        {
            return null;
        }

        public RestifySearchResult Search(string text)
        {
            return SpotifyManager.Current.Search(text);
        }
    }
}
