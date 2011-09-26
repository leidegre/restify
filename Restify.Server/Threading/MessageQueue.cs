using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Linq;
using System.Text;
using System.Threading;

namespace Restify.Threading
{
    [Export(typeof(IMessageQueue))]
    public class MessageQueue : IMessageQueue
    {
        class QuitMessage : IMessage
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
                throw new InvalidOperationException("Cannot post message from thread that is running the message loop. This will result a deadlock.");
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
