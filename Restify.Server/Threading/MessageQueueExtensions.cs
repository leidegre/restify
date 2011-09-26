using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace Restify.Threading
{
    public static class MessageQueueExtensions
    {
        class SynchronizedMessage : IMessage
        {
            private IMessage msg;
            private ManualResetEventSlim wait;

            public SynchronizedMessage(IMessage msg)
            {
                this.msg = msg;
                this.wait = new ManualResetEventSlim(false);
            }

            public void Invoke(IDictionary<string, object> state)
            {
                try
                {
                    msg.Invoke(state);
                }
                finally
                {
                    wait.Set();
                }
            }

            public void Wait()
            {
                wait.Wait();
            }
        }

        public static void PostSynchronized(this IMessageQueue q, IMessage msg)
        {
            if (msg == null)
            {
                throw new ArgumentNullException("msg");
            }
            var synchronizedMessage = new SynchronizedMessage(msg);
            q.Post(synchronizedMessage);
            synchronizedMessage.Wait();
        }
    }
}
