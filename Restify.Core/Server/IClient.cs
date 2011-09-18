using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Restify.ServiceModel;

namespace Restify.Server
{
    public interface IClient : IServiceProvider
    {
        string Id { get; }
        void Initialize(string agentId);
    }
}
