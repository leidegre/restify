using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Linq;
using System.ServiceModel.Web;
using System.Text;

namespace Restify.ServiceModel.Composition
{
    [PartNotDiscoverable]
    [Export(typeof(IServiceContext))]
    class ComposableServiceContext : IServiceContext
    {
        private WebOperationContext webOperationContext;

        public ComposableServiceContext(WebOperationContext webOperationContext)
        {
            this.webOperationContext = webOperationContext;
        }

        public string GetParameter(string parameterName)
        {
            var incomingRequest = webOperationContext.IncomingRequest;
            return incomingRequest.Headers[parameterName];
        }
    }
}
