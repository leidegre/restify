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
        [WebInvoke(UriTemplate = "/", RequestFormat = WebMessageFormat.Json, ResponseFormat = WebMessageFormat.Json)]
        bool Login(RestifyLogin login);
    }
}
