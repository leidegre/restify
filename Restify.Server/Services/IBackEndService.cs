using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.Text;
using System.ServiceModel.Web;

namespace Restify.Services
{
    [ServiceContract]
    public interface IBackEndService
    {
        [OperationContract]
        [WebInvoke(UriTemplate = "/is-logged-in")]
        RestifyLoginResponse IsLoggedIn(RestifyLoginRequest login);

        [OperationContract]
        [WebInvoke(UriTemplate = "/login")]
        RestifyLoginResponse Login(RestifyLoginRequest login);

        [OperationContract]
        [WebInvoke(UriTemplate = "/playlists")]
        List<RestifyPlaylist> GetPlaylists();

        [OperationContract]
        [WebInvoke(UriTemplate = "/playlist")]
        List<RestifyTrack> GetPlaylist(RestifyPlaylistRequest playlist);

        [OperationContract]
        [WebInvoke(UriTemplate = "/play")]
        void Play(RestifyTrackRequest track);

        [OperationContract]
        [WebInvoke(Method = "PUT", UriTemplate = "/queue/{trackId}")]
        void Enqueue(string trackId);
    }
}
