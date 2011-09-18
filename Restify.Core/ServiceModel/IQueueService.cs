using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel.Web;
using System.ServiceModel;

namespace Restify.ServiceModel
{
    [ServiceContract]
    public interface IQueueService : IDisposable
    {
        [OperationContract]
        [WebInvoke(UriTemplate = "/{trackId}")]
        void Enqueue(string trackId);

        [OperationContract]
        [WebInvoke(Method = "DELETE", UriTemplate = "/")]
        string Dequeue();
    }
}
