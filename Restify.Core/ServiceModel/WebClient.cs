using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel;
using System.ServiceModel.Description;
using System.ServiceModel.Web;

namespace Restify.ServiceModel
{
    public abstract class WebClient<TChannel> : ClientBase<TChannel>
        where TChannel : class
    {
        protected WebClient(Uri address)
            : base(new WebHttpBinding(), new EndpointAddress(address))
        {
            Endpoint.Behaviors.Add(new WebHttpBehavior {
                AutomaticFormatSelectionEnabled = false,
                DefaultOutgoingRequestFormat = WebMessageFormat.Json,
                DefaultOutgoingResponseFormat = WebMessageFormat.Json,
                FaultExceptionEnabled = true,
            });
        }
    }
}
