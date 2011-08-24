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
        [WebGet(UriTemplate = "/status", ResponseFormat = WebMessageFormat.Json)]
        RestifyViewModel GetStatus();

        [OperationContract]
        [WebInvoke(UriTemplate = "/is-logged-in", RequestFormat = WebMessageFormat.Json, ResponseFormat = WebMessageFormat.Json)]
        RestifyLoginResponse IsLoggedIn(RestifyLogin login);

        [OperationContract]
        [WebInvoke(UriTemplate = "/login", RequestFormat = WebMessageFormat.Json, ResponseFormat = WebMessageFormat.Json)]
        bool Login(RestifyLogin login);

        [OperationContract]
        [WebGet(UriTemplate = "/playlists", ResponseFormat = WebMessageFormat.Json)]
        List<RestifyPlaylist> GetPlaylists();
    }
}
