using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel;
using System.ServiceModel.Description;
using System.ServiceModel.Web;

namespace Restify.ServiceModel
{
    class QueueServiceClient : WebClient<IQueueService>, IQueueService
    {
        public QueueServiceClient(Uri queueUri)
            : base(queueUri)
        {
        }

        public void Enqueue(string trackId)
        {
            using (new OperationContextScope(InnerChannel))
            {
                Channel.Enqueue(trackId);
            }
        }

        public string Dequeue()
        {
            using (new OperationContextScope(InnerChannel))
            {
                return Channel.Dequeue();
            }
        }
    }
}
