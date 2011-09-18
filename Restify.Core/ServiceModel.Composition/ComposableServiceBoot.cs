using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel;
using System.ComponentModel.Composition.Primitives;
using System.ComponentModel.Composition.Hosting;
using System.ComponentModel.Composition;
using System.Diagnostics;

namespace Restify.ServiceModel.Composition
{
    public class ComposableServiceBoot : IDisposable
    {
        private CompositionContainer container;
        
        public Uri BaseEndpoint { get; private set; }
        
        private List<ServiceHost> serviceHosts = new List<ServiceHost>();

        public ComposableServiceBoot(CompositionContainer container, Uri baseEndpoint)
        {
            this.container = container;
            this.BaseEndpoint = baseEndpoint;
        }

        public void Open()
        {
            if (serviceHosts.Count > 0)
                throw new InvalidOperationException("Open can only be called once.");

            var serviceExports = container.GetExports(new ContractBasedImportDefinition(ComposableServiceExportAttribute.ComposableServiceContractName
                , null
                , new[] { new KeyValuePair<string, Type>("Address", typeof(string)), new KeyValuePair<string, Type>("ServiceType", typeof(Type)) }
                , ImportCardinality.ZeroOrMore
                , false
                , false
                , CreationPolicy.Any
                ));

            serviceExports.AsParallel().ForAll(serviceExport => {

                var address = (string)serviceExport.Metadata["Address"];

                // Allow the address to be rewritten based on property exports
                // this allows us to have simple contextual end points
                // as each agent runs in it's own "namespace" this is very useful
                var addressMetadata = (string[])serviceExport.Metadata["AddressMetadata"];
                if (addressMetadata != null)
                    address = string.Format(address, (from x in addressMetadata select container.GetExportedValue<object>(x)).ToArray());

                var serviceEndpoint = new Uri(BaseEndpoint, address);

                var serviceType = (Type)serviceExport.Metadata["ServiceType"];
                var serviceTypeIdentity = AttributedModelServices.GetTypeIdentity(serviceType);
                if (serviceTypeIdentity != (string)serviceExport.Metadata["ExportTypeIdentity"])
                    throw new InvalidOperationException(string.Format("The ExportTypeIdentity '{0}' doesn't match the service type identity '{1}'.", (string)serviceExport.Metadata["ExportTypeIdentity"], serviceTypeIdentity));

                Trace.WriteLine(string.Format("Open {0}", serviceEndpoint));
                var serviceHost = new ComposableServiceHost(container, serviceExport.Definition.ContractName, serviceType, new[] { serviceEndpoint });
                serviceHost.Open();

                serviceHosts.Add(serviceHost);

            });
        }

        public void Dispose()
        {
            foreach (var serviceHost in serviceHosts)
            {
                ((IDisposable)serviceHost).Dispose();
            }
            serviceHosts.Clear();
        }
    }
}
