using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel;
using System.ServiceModel.Description;
using System.ServiceModel.Web;

namespace Restify.ServiceModel
{
    public class BackEndServiceClient : WebClient<IClientService>, IClientService
    {
        public BackEndServiceClient(Uri address)
            : base(address)
        {
        }

        public RestifySearchResult Search(string text)
        {
            using (new OperationContextScope(InnerChannel))
            {
                return Channel.Search(text);
            }
        }

        public void Play()
        {
            using (new OperationContextScope(InnerChannel))
            {
                Channel.Play();
            }
        }

        public void PlayPause()
        {
            using (new OperationContextScope(InnerChannel))
            {
                Channel.PlayPause();
            }
        }

        public void Next()
        {
            using (new OperationContextScope(InnerChannel))
            {
                Channel.Next();
            }
        }
    }
}
