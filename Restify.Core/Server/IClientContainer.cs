using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Restify.ServiceModel;

namespace Restify.Server
{
    public interface IClientContainer
    {
        IClient Create(string agentId);
        void Remove(IClient agent);
        void Play();
    }
}
