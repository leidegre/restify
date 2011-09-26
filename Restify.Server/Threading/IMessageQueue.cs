using System;
namespace Restify.Threading
{
    public interface IMessageQueue
    {
        void Post(IMessage msg);
        void PostQuit(int exitCode);
        int RunMessageLoop();
    }
}
