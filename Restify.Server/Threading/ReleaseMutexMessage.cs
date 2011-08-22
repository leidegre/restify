using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace Restify.Threading
{
    public class ReleaseMutexMessage : IMessage
    {
        private Mutex instance;

        public ReleaseMutexMessage(Mutex instance)
        {
            if (instance == null)
            {
                throw new ArgumentNullException("instance");
            }
            this.instance = instance;
        }

        public void Invoke(IDictionary<string, object> state)
        {
            instance.ReleaseMutex();
        }
    }
}
