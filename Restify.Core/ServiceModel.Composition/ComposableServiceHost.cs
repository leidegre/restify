using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel.Web;
using System.ComponentModel.Composition.Hosting;
using System.ServiceModel.Description;

namespace Restify.ServiceModel.Composition
{
    public class ComposableServiceHost : WebServiceHost
    {
        private CompositionContainer container;
        private string contractName;

        public ComposableServiceHost(CompositionContainer container, string contractName, Type serviceType, Uri[] baseAddresses)
            : base(serviceType, baseAddresses)
        {
            this.container = container;
            this.contractName = contractName;
            this.Opening += new EventHandler(ComposableServiceHostOpening);
        }

        void ComposableServiceHostOpening(object sender, EventArgs e)
        {
            foreach (var serviceEndpoint in Description.Endpoints)
            {
                var webHttpBehavior = serviceEndpoint.Behaviors.Find<WebHttpBehavior>();
                if (webHttpBehavior != null)
                {
                    webHttpBehavior.DefaultOutgoingRequestFormat = WebMessageFormat.Json;
                    webHttpBehavior.DefaultOutgoingResponseFormat = WebMessageFormat.Json;
                    webHttpBehavior.FaultExceptionEnabled = true;
                    webHttpBehavior.HelpEnabled = true;
                    webHttpBehavior.AutomaticFormatSelectionEnabled = false;
                    webHttpBehavior.DefaultBodyStyle = WebMessageBodyStyle.Bare;
                }
                else
                    serviceEndpoint.Behaviors.Add(new WebHttpBehavior {
                        DefaultOutgoingRequestFormat = WebMessageFormat.Json,
                        DefaultOutgoingResponseFormat = WebMessageFormat.Json,
                        FaultExceptionEnabled = true,
                        HelpEnabled = true,
                        AutomaticFormatSelectionEnabled = false,
                        DefaultBodyStyle = WebMessageBodyStyle.Bare,
                    });
            }
            
            var serviceDebugBehavior = Description.Behaviors.Find<ServiceDebugBehavior>();
            if (serviceDebugBehavior != null)
            {
                serviceDebugBehavior.HttpHelpPageEnabled = true;
                serviceDebugBehavior.IncludeExceptionDetailInFaults = true;
            }
            else
                Description.Behaviors.Add(new ServiceDebugBehavior { 
                    HttpHelpPageEnabled = true,
                    IncludeExceptionDetailInFaults = true,
                });

            var serviceMetadataBehavior = Description.Behaviors.Find<ServiceMetadataBehavior>();
            if (serviceMetadataBehavior != null)
                serviceMetadataBehavior.HttpGetEnabled = true;
            else
                Description.Behaviors.Add(new ServiceMetadataBehavior {
                    HttpGetEnabled = true,
                });
        }

        protected override void OnOpening()
        {
            if (Description == null)
            {
                return;
            }
            Description.Behaviors.Add(new ComposableServiceBehavior(container, contractName, Description.ServiceType));
            base.OnOpening();
        }
    }
}
