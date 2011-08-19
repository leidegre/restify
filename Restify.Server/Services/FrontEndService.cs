using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.Text;
using System.IO;
using System.ServiceModel.Web;

namespace Restify.Services
{
    // NOTE: You can use the "Rename" command on the "Refactor" menu to change the class name "FrontEndService" in both code and config file together.
    public class FrontEndService : IFrontEndService
    {
        public Stream Index()
        {
            return Content("index.htm");
        }

        public Stream Content(string file)
        {
            var ext = Path.GetExtension(file);
            switch (ext)
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
                    WebOperationContext.Current.OutgoingResponse.ContentType = "text/html";
                    break;
            }
            return File.OpenRead(Path.Combine(@"..\..\Content", file));
        }
    }
}
