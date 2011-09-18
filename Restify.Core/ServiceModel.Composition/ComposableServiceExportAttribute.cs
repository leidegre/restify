using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel.Composition;

namespace Restify.ServiceModel.Composition
{
    [MetadataAttribute]
    [AttributeUsage(AttributeTargets.Class, AllowMultiple = false)]
    public class ComposableServiceExportAttribute : ExportAttribute, IComposableServiceMetadata
    {
        public const string ComposableServiceContractName = "ComposableService";

        public string Address { get; private set; }
        public string[] AddressMetadata { get; set; }

        public Type ServiceType { get; private set; }

        public ComposableServiceExportAttribute(string address, Type serviceType)
            : base(ComposableServiceContractName, null)
        {
            if (address == null)
                throw new ArgumentNullException("address");
            if (serviceType == null)
                throw new ArgumentNullException("serviceType");
            if (!serviceType.IsClass)
                throw new ArgumentNullException("Only class service types are supported by the ServiceHost.", "serviceType");
            this.Address = address;
            this.ServiceType = serviceType;
        }
    }
}
