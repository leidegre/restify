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
        [WebInvoke(UriTemplate = "/search?q={text}")]
        RestifySearchResult Search(string text);

        [OperationContract]
        [WebInvoke(UriTemplate = "/playback/play")]
        void Play();

        [OperationContract]
        [WebInvoke(UriTemplate = "/playback/playPause")]
        void PlayPause();

        [OperationContract]
        [WebInvoke(UriTemplate = "/playback/next")]
        void Next();

        [OperationContract]
        [WebInvoke(Method = "PUT", UriTemplate = "/queue/{trackId}")]
        void Enqueue(string trackId);

        [OperationContract]
        [WebInvoke(Method = "DELETE", UriTemplate = "/queue")]
        RestifyTrack Dequeue();
    }
}
