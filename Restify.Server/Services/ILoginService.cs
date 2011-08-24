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
    public interface ILoginService
    {
        [OperationContract]
        [WebInvoke(UriTemplate = "/status", RequestFormat = WebMessageFormat.Json, ResponseFormat = WebMessageFormat.Json)]
        RestifyLoginResponse Query(RestifyLogin login);

        [OperationContract]
        [WebInvoke(UriTemplate = "/login", RequestFormat = WebMessageFormat.Json, ResponseFormat = WebMessageFormat.Json)]
        RestifyLoginResponse Login(RestifyLogin login);
    }
}
