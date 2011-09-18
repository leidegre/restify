using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.Text;
using System.ServiceModel.Web;

namespace Restify.ServiceModel
{
    [ServiceContract]
    public interface IClientService : IDisposable
    {
        [OperationContract]
        [WebInvoke(UriTemplate = "/search?q={text}")]
        RestifySearchResult Search(string text);

        [OperationContract]
        [WebInvoke(UriTemplate = "/playback/play")]
        void Play();

        [OperationContract]
        [WebInvoke(UriTemplate = "/playback/playPause")]
        void PlayPause();

        [OperationContract]
        [WebInvoke(UriTemplate = "/playback/next")]
        void Next();
    }
}
