using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Diagnostics.Contracts;

namespace Restify.Threading
{
    public static class ThreadContext
    {
        public static void Invoke(Action action)
        {
            Contract.Requires(action != null);
            Exception ex = null;
            using (var waitEvent = new ManualResetEventSlim(false))
            {
                ThreadPool.QueueUserWorkItem(_ => {
                    try
                    {
                        action();
                    }
                    catch (Exception innerException)
                    {
                        ex = innerException;
                    }
                    finally
                    {
                        waitEvent.Set();
                    }
                });
                waitEvent.Wait();
            }
            if (ex != null)
            {
                throw new InvalidOperationException(ex.Message, ex);
            }
        }
    }
}
