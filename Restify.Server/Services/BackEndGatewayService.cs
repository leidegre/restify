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

        public void Play(RestifyTrackRequest track)
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
                        client.Play(track);
                    }
                }
            }
        }

        public void Enqueue(string trackId)
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
                        client.Enqueue(trackId);
                    }
                }
            }
        }
    }
}
