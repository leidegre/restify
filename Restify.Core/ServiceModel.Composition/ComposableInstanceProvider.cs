using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel.Dispatcher;
using System.ComponentModel.Composition.Hosting;
using System.ComponentModel.Composition.Primitives;
using System.ComponentModel.Composition;

namespace Restify.ServiceModel.Composition
{
    public class ComposableInstanceProvider : IInstanceProvider
    {
        class AttributedPartCatalog : ComposablePartCatalog
        {
            IQueryable<ComposablePartDefinition> parts;

            public override IQueryable<ComposablePartDefinition> Parts
            {
                get { return parts; }
            }

            public AttributedPartCatalog(params Type[] types)
            {
                this.parts = (from type in types select AttributedModelServices.CreatePartDefinition(type, null)).ToList().AsQueryable();
            }
        }

        private CompositionContainer container;
        private ContractBasedImportDefinition importDefinition;

        public ComposableInstanceProvider(CompositionContainer container, string contractName, Type serviceType)
        {
            this.container = container;
            var typeIdentity = AttributedModelServices.GetTypeIdentity(serviceType);
            this.importDefinition = new ContractBasedImportDefinition(contractName, typeIdentity, null, ImportCardinality.ExactlyOne, false, true, CreationPolicy.NonShared);
        }

        private Export GetExport(CompositionContainer container)
        {
            return container.GetExports(importDefinition).First();
        }

        public object GetInstance(System.ServiceModel.InstanceContext instanceContext, System.ServiceModel.Channels.Message message)
        {
            return GetInstance(instanceContext);
        }

        public object GetInstance(System.ServiceModel.InstanceContext instanceContext)
        {
            var catalog2 = new AttributedPartCatalog(typeof(ComposableServiceContext));
            var container2 = new CompositionContainer(new AggregateCatalog(catalog2, container.Catalog));
            container2.ComposeParts(new ComposableServiceContext(System.ServiceModel.Web.WebOperationContext.Current));
            var export = GetExport(container2);
            return export.Value;
        }

        public void ReleaseInstance(System.ServiceModel.InstanceContext instanceContext, object instance)
        {
            var disposable = instance as IDisposable;
            if (disposable != null)
                disposable.Dispose();
        }
    }
}
