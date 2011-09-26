using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel;
using System.ServiceModel.Web;
using System.Diagnostics;
using Restify.ServiceModel;
using Restify.ServiceModel.Composition;
using System.ComponentModel.Composition;
using Restify.Server;

namespace Restify.ServiceModel
{
    [ComposableServiceExport("/restify/master", typeof(BackEndGatewayService))]
    public class BackEndGatewayService : IClientService
    {
        private IClientContainer container;

        [ImportingConstructor]
        public BackEndGatewayService(IClientContainer container)
        {
            this.container = container;
        }

        public RestifySearchResult Search(string text)
        {
            throw new NotImplementedException();
        }

        public void Play()
        {
            var agent = container.GetPlayToken();
            using (var client = agent.GetService<IClientService>())
                client.Play();
        }

        public void PlayPause()
        {
            var agent = container.GetPlayToken();
            using (var client = agent.GetService<IClientService>())
                client.PlayPause();
        }

        public void Next()
        {
            var agent = container.GetPlayToken();
            using (var client = agent.GetService<IClientService>())
                client.Next();
        }

        void IDisposable.Dispose()
        {
        }
    }
}
