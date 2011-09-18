using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.Text;
using System.ServiceModel.Web;
using System.IO;

namespace Restify.ServiceModel
{
    [ServiceContract]
    public interface IFrontEndService
    {
        [OperationContract]
        [WebGet(UriTemplate = "/")]
        Stream Index();

        [OperationContract]
        [WebGet(UriTemplate = "/{file}")]
        Stream Content(string file);
    }
}
