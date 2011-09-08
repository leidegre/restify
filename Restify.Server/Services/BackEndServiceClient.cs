using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel;
using System.ServiceModel.Description;
using System.ServiceModel.Web;

namespace Restify.Services
{
    public class BackEndServiceClient : ClientBase<IBackEndService>, IBackEndService
    {
        public BackEndServiceClient(string address)
            : base(new WebHttpBinding(), new EndpointAddress(address))
        {
            Endpoint.Behaviors.Add(new WebHttpBehavior {
                AutomaticFormatSelectionEnabled = false,
                DefaultBodyStyle = WebMessageBodyStyle.Bare,
                DefaultOutgoingRequestFormat = WebMessageFormat.Json,
                DefaultOutgoingResponseFormat = WebMessageFormat.Json,
                FaultExceptionEnabled = true,
            });
        }

        #region Only a subset of the backend is used through a proxy

        public RestifyLoginResponse IsLoggedIn(RestifyLoginRequest login)
        {
            using (new OperationContextScope(InnerChannel))
            {
                return Channel.IsLoggedIn(login);
            }
        }

        public RestifyLoginResponse Login(RestifyLoginRequest login)
        {
            using (new OperationContextScope(InnerChannel))
            {
                return Channel.Login(login);
            }
        }

        #endregion

        public List<RestifyPlaylist> GetPlaylists()
        {
            throw new NotImplementedException();
        }

        public List<RestifyTrack> GetPlaylist(RestifyPlaylistRequest playlist)
        {
            throw new NotImplementedException();
        }

        public void Play()
        {
            using (new OperationContextScope(InnerChannel))
            {
                Channel.Play();
            }
        }

        public void PlayPause()
        {
            using (new OperationContextScope(InnerChannel))
            {
                Channel.PlayPause();
            }
        }

        public void Next()
        {
            using (new OperationContextScope(InnerChannel))
            {
                Channel.Next();
            }
        }

        public void Enqueue(string trackId)
        {
            using (new OperationContextScope(InnerChannel))
            {
                Channel.Enqueue(trackId);
            }
        }

        public RestifyTrack Dequeue()
        {
            using (new OperationContextScope(InnerChannel))
            {
                return Channel.Dequeue();
            }
        }

        public RestifySearchResult Search(string text)
        {
            using (new OperationContextScope(InnerChannel))
            {
                return Channel.Search(text);
            }
        }
    }
}
