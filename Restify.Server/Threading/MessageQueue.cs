using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace Restify.Threading
{
    public class MessageQueue
    {
        public class QuitMessage : IMessage
        {
            public int ExitCode { get; private set; }

            public QuitMessage(int exitCode)
            {
                ExitCode = exitCode;
            }

            public void Invoke(IDictionary<string, object> state)
            {
            }
        }

        public class SynchronizedMessage : IMessage
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

        BlockingCollection<IMessage> queue;

        public MessageQueue()
        {
            queue = new BlockingCollection<IMessage>();
        }

#if DEBUG
        [System.Runtime.InteropServices.DllImport("kernel32.dll")]
        static extern int GetCurrentThreadId();

        int messageLoopThreadId;
#endif

        public void Post(IMessage msg)
        {
#if DEBUG
            if (GetCurrentThreadId() == messageLoopThreadId)
            {
                throw new InvalidOperationException("Cannot post message from thread that is running the message loop. This can result in a deadlock.");
            }
#endif
            if (msg == null)
            {
                throw new ArgumentNullException("msg");
            }
            queue.Add(msg);
        }

        public void PostQuit(int exitCode)
        {
            queue.Add(new QuitMessage(exitCode));
        }

        public void PostSynchronized(IMessage msg)
        {
            if (msg == null)
            {
                throw new ArgumentNullException("msg");
            }
            var synchronizedMessage = new SynchronizedMessage(msg);
            Post(synchronizedMessage);
            synchronizedMessage.Wait();
        }

        public int RunMessageLoop()
        {
#if DEBUG
            messageLoopThreadId = GetCurrentThreadId();
#endif
            var state = new Dictionary<string, object>(StringComparer.Ordinal);
            for (; ; )
            {
                var msg = queue.Take();
                if (msg is QuitMessage)
                    return ((QuitMessage)msg).ExitCode;
                msg.Invoke(state);
            }
        }
    }
}
