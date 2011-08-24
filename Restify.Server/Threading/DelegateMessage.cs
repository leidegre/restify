using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics.Contracts;

namespace Restify.Threading
{
    public class DelegateMessage : IMessage
    {
        private Action<IDictionary<string, object>> action;

        public DelegateMessage(Action<IDictionary<string, object>> action)
        {
            Contract.Requires(action != null); 
            this.action = action;
        }

        public void Invoke(IDictionary<string, object> state)
        {
            action(state);
        }
    }
}
