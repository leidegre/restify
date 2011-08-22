using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify.Threading
{
    public interface IMessage
    {
        void Invoke(IDictionary<string, object> state);
    }
}
