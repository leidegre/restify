using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace Restify.Threading
{
    public class CreateMutexMessage : IMessage
    {
        public string InstanceName { get; private set; }

        bool createdNew;
        public bool CreatedNew { get { return createdNew; } }
        
        public Mutex Instance { get; private set; }

        public CreateMutexMessage(string instanceName)
        {
            InstanceName = instanceName;
        }

        public void Invoke(IDictionary<string, object> state)
        {
            Instance = new Mutex(true, InstanceName, out createdNew);
        }
    }
}
