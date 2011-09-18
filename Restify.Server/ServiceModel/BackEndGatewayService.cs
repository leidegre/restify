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
        public RestifySearchResult Search(string text)
        {
            throw new NotImplementedException();
        }

        public void Play()
        {
            throw new NotImplementedException();
        }

        public void PlayPause()
        {
            throw new NotImplementedException();
        }

        public void Next()
        {
            throw new NotImplementedException();
        }

        void IDisposable.Dispose()
        {
        }
    }
}
