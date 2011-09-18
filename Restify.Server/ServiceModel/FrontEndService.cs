using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.Text;
using System.IO;
using System.ServiceModel.Web;
using Restify.ServiceModel;
using Restify.ServiceModel.Composition;
using System.ComponentModel.Composition;

namespace Restify.ServiceModel
{
    [ComposableServiceExport("/restify", typeof(FrontEndService))]
    public class FrontEndService : IFrontEndService
    {
        private IEnvironment environment;

        [ImportingConstructor]
        public FrontEndService(IEnvironment environment)
        {
            this.environment = environment;
        }

        public Stream Index()
        {
            return Content("index.htm");
        }

        public Stream Content(string file)
        {
            var extension = Path.GetExtension(file);
            switch (extension)
            {
                case ".htm":
                    WebOperationContext.Current.OutgoingResponse.ContentType = "text/html";
                    break;
                case ".css":
                    WebOperationContext.Current.OutgoingResponse.ContentType = "text/css";
                    break;
                case ".png":
                    WebOperationContext.Current.OutgoingResponse.ContentType = "image/png";
                    break;
                case ".js":
                    WebOperationContext.Current.OutgoingResponse.ContentType = "text/javascript";
                    break;
                default:
                    WebOperationContext.Current.OutgoingResponse.ContentType = "application/octet-stream";
                    break;
            }
            var path = environment.GetFile(environment.GetVirtualPath("Content", file));
            if (path.IsValid)
            {
                return environment.GetStream(path);
            }
            return null;
        }
    }
}
