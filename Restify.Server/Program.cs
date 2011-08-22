using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.ServiceModel;
using System.ServiceModel.Description;
using System.Text;
using System.Threading;
using Restify.Services;
using System.Collections.Concurrent;
using Restify.Threading;
using System.Diagnostics;

namespace Restify
{
    class Program
    {
        [DllImport("kernel32.dll")]
        extern static bool SetDllDirectory(string lpPathName);

        static int Main(string[] args)
        {
            // This call cannot be made in a method that uses
            // any reference to the "Restify.Spotify" wrapper
            // it will trigger a load of the "libspotify.dll" 
            // which if not found will crash the app
            // "libspotify.dll" has to be found in the DLL search paths
            var dllSerachPath = Path.GetFullPath(@"..\..\..\lib");
            SetDllDirectory(dllSerachPath);

            string instanceName = null;

            for (int i = 0; i < args.Length; i++)
            {
                if ("/InstanceName".Equals(args[i], StringComparison.OrdinalIgnoreCase))
                {
                    if (!(i + 1 < args.Length))
                    {
                        Console.Error.WriteLine("Option /InstanceName missing argument: instanceName");
                        return 1;
                    }
                    instanceName = args[++i];
                }
            }

            if (string.IsNullOrEmpty(instanceName))
            {
                // run front end and gateway
                return Run();
            }
            else
            {
                return RunInstance(instanceName);
            }
        }

        public static MessageQueue MessageQueue { get; private set; }

        public static void Post(IMessage msg)
        {
            MessageQueue.Post(msg);
        }

        public static void PostSynchronized(IMessage msg)
        {
            MessageQueue.PostSynchronized(msg);
        }

        private static int Run()
        {
            MessageQueue = new MessageQueue();

            Console.WriteLine("Configuring service host...");

            var frontEndHost = new ServiceHost(typeof(FrontEndService));
            var frontEndBaseUri = "http://localhost/restify";
            var frontEndEndPoint = frontEndHost.AddServiceEndpoint(typeof(IFrontEndService), new WebHttpBinding(), frontEndBaseUri);
            frontEndEndPoint.Behaviors.Add(new WebHttpBehavior());
            frontEndHost.Open();

            var loginHost = new ServiceHost(typeof(LoginService));
            var loginBaseUri = "http://localhost/restify/login";
            var loginEndPoint = loginHost.AddServiceEndpoint(typeof(ILoginService), new WebHttpBinding(), loginBaseUri);
            loginEndPoint.Behaviors.Add(new WebHttpBehavior());
            loginHost.Open();

            Console.WriteLine("OK");

            ThreadPool.QueueUserWorkItem(_ => {
                Console.WriteLine("Press 'S' to shutdown server...");
                while (Console.ReadKey(true).Key != ConsoleKey.S)
                    ;
                lock (LoginService.userMapping)
                {
                    foreach (var item in LoginService.userMapping)
                    {
                        item.Value.Shutdown();
                    }
                }
                MessageQueue.PostQuit(0);
            });

            System.Diagnostics.Process.Start("http://localhost/restify");

            return MessageQueue.RunMessageLoop();
        }

        private static int RunInstance(string instanceName)
        {
            if (Debugger.IsAttached)
                Debugger.Break();

            var exitCode = 0;
            using (var backEndHost = new ServiceHost(typeof(BackEndService)))
            {
                try
                {
                    var backEndBaseUri = "http://localhost/restify/user/" + instanceName;
                    var backEndEndPoint = backEndHost.AddServiceEndpoint(typeof(IBackEndService), new WebHttpBinding { Security = new WebHttpSecurity { Mode = WebHttpSecurityMode.None } }, backEndBaseUri);
                    backEndEndPoint.Behaviors.Add(new WebHttpBehavior());
                    backEndHost.Open();
                }
                catch (Exception ex)
                {
                    Console.Error.WriteLine("{0}: {1}", ex.GetType(), ex.Message);
                    exitCode = 1;
                }
                finally
                {
                    // Signal that the instance is running
                    var s = string.Format("Global\\{0}Init", instanceName);
                    bool createdNew;
                    using (var m = new Semaphore(0, 1, s, out createdNew))
                    {
                        if (!createdNew)
                            m.Release();
                    }
                }

                if (exitCode == 0)
                {
                    // Block thread, either by an existing system-wide Mutex
                    // or wait for a key press
                    var s = string.Format("Global\\{0}Exit", instanceName);
                    bool createdNew;
                    using (var exit = new Mutex(false, s, out createdNew))
                    {
                        if (!createdNew)
                            try
                            {
                                exit.WaitOne();
                            }
                            catch (AbandonedMutexException)
                            {
                                // this should only happen if the parent process is killed
                                // and cannot shutdown gracefully
                                return 1;
                            }
                    }

                    if (createdNew)
                    {
                        Console.WriteLine("Press 'S' to shutdown this instance...");
                        while (Console.ReadKey(true).Key != ConsoleKey.S)
                            ;
                    }
                }
            }
            return exitCode;
        }
    }
}
