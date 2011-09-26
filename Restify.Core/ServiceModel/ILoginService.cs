using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.ServiceModel.Web;
using System.Text;

namespace Restify.ServiceModel
{
    [ServiceContract]
    public interface ILoginService : IDisposable
    {
        [OperationContract]
        [WebGet(UriTemplate = "/login/ping")]
        string Ping();

        [OperationContract]
        [WebInvoke(UriTemplate = "/login")]
        LoginResponse Login(LoginRequest login);
    }
}
