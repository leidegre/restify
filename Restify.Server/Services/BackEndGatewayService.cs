using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel;
using System.ServiceModel.Web;

namespace Restify.Services
{
    public class BackEndGatewayService : IBackEndService
    {
        const string InstanceHeaderName = "X-RESTify-Instance";

        public RestifyLoginResponse IsLoggedIn(RestifyLoginRequest login)
        {
            throw new NotImplementedException();
        }

        public RestifyLoginResponse Login(RestifyLoginRequest login)
        {
            throw new NotImplementedException();
        }

        public List<RestifyPlaylist> GetPlaylists()
        {
            throw new NotImplementedException();
        }

        public List<RestifyTrack> GetPlaylist(RestifyPlaylistRequest playlist)
        {
            throw new NotImplementedException();
        }

        public void Play(bool play)
        {
            lock (LoginService.userMapping)
            {
                var instance = LoginService.userMapping.FirstOrDefault(x => x.Value.IsMaster).Value;
                if (instance == null)
                {
                    instance = LoginService.userMapping.FirstOrDefault().Value;
                    if (instance != null)
                        instance.IsMaster = true;
                }
                if (instance != null)
                {
                    using (var client = instance.CreateClient())
                    {
                        client.Play(play);
                    }
                }
            }
        }

        public RestifySearchResult Search(string text)
        {
            throw new NotImplementedException();
        }

        static Queue<string> pq = new Queue<string>();
        static HashSet<string> pqset = new HashSet<string>(StringComparer.Ordinal);

        public void Enqueue(string trackId)
        {
            lock (pq)
            {
                // only accept currently not queued tracks
                if (pqset.Add(trackId))
                    pq.Enqueue(trackId);
                else
                    return;
            }

            lock (LoginService.userMapping)
            {
                var instance = LoginService.userMapping.FirstOrDefault(x => x.Value.IsMaster).Value;
                if (instance == null)
                {
                    instance = LoginService.userMapping.FirstOrDefault().Value;
                    if (instance != null)
                        instance.IsMaster = true;
                }
                if (instance != null)
                {
                    using (var client = instance.CreateClient())
                    {
                        client.Play(true);
                    }
                }
            }
        }

        public RestifyTrack Dequeue()
        {
            lock (pq)
            {
                if (pq.Count > 0)
                {
                    var trackId = pq.Dequeue();
                    pqset.Remove(trackId);
                    return new RestifyTrack { Id = trackId };
                }
            }
            return null;
        }
    }
}
