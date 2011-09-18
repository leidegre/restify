using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify.Data
{
    public interface IPersistentService
    {
        void Initialize();
        void Enqueue(string userName, string tarckId);
        string Dequeue();
    }
}
