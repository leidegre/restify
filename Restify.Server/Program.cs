using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.ComponentModel;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using System.ComponentModel.Composition.Primitives;
using System.Data.SqlServerCe;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.ServiceModel;
using System.ServiceModel.Channels;
using System.ServiceModel.Description;
using System.ServiceModel.Dispatcher;
using System.ServiceModel.Web;
using System.Text;
using System.Threading;
using Restify.ServiceModel;
using Restify.ServiceModel;
using Restify.ServiceModel.Composition;
using Restify.Threading;
using Restify.Server;

namespace Restify
{
    public class Program
    {
        [DllImport("kernel32.dll")]
        extern static IntPtr GetConsoleWindow();

        [DllImport("kernel32.dll")]
        extern static bool SetDllDirectory(string lpPathName);

        [Export(ConfigurationExports.BaseEndpoint)]
        public static Uri BaseEndpoint { get; private set; }

        static Program()
        {
            BaseEndpoint = new Uri("http://localhost");
        }

        static int Main(string[] args)
        {
            // This call cannot be made in a method that uses
            // any reference to the "Restify.Spotify" wrapper
            // it will trigger a load of the "spotify.dll" 
            // which if not found will crash the app
            // "spotify.dll" has to be found in the DLL search paths
            SetDllDirectory(Path.GetFullPath(@"..\..\..\lib"));

            // If launched with a console window 
            // (this is not the case when running as a service or when creating child processes)
            if (GetConsoleWindow() != IntPtr.Zero)
                Trace.Listeners.Add(new ConsoleTraceListener());

            Trace.WriteLine(string.Format("CurrentDirectory: {0}", Environment.CurrentDirectory));

            return RunController();
        }

        class AccessControlDispatchMessageInspector : IDispatchMessageInspector
        {
            public object AfterReceiveRequest(ref System.ServiceModel.Channels.Message request, IClientChannel channel, InstanceContext instanceContext)
            {
                //var instanceName = WebOperationContext.Current.IncomingRequest.Headers["X-RESTify-Instance"];
                //lock (LoginService.userMapping)
                //{
                //    if (!LoginService.userInstanceMapping.ContainsKey(instanceName))
                //    {
                //        throw new InvalidOperationException();
                //    }
                //}
                return null;
            }

            public void BeforeSendReply(ref System.ServiceModel.Channels.Message reply, object correlationState)
            {
            }
        }

        class AccessControlEndpointBehavior : IEndpointBehavior
        {
            public void AddBindingParameters(ServiceEndpoint endpoint, System.ServiceModel.Channels.BindingParameterCollection bindingParameters)
            {
            }

            public void ApplyClientBehavior(ServiceEndpoint endpoint, System.ServiceModel.Dispatcher.ClientRuntime clientRuntime)
            {
            }

            public void ApplyDispatchBehavior(ServiceEndpoint endpoint, System.ServiceModel.Dispatcher.EndpointDispatcher endpointDispatcher)
            {
                endpointDispatcher.DispatchRuntime.MessageInspectors.Add(new AccessControlDispatchMessageInspector());
            }

            public void Validate(ServiceEndpoint endpoint)
            {
            }
        }

        public static int RunController()
        {
            Trace.WriteLine("Configuring data store...");

            var catalog = new AggregateCatalog(new AssemblyCatalog(typeof(Program).Assembly), new DirectoryCatalog(Environment.CurrentDirectory));

            var container = new CompositionContainer(catalog);

            container.GetExportedValue<Data.IPersistentService>()
                .Initialize();

            Trace.WriteLine("OK");

            var messageQueue = container.GetExportedValue<IMessageQueue>();

            using (var composableServiceBoot = new ComposableServiceBoot(container, BaseEndpoint))
            {
                Trace.WriteLine("Configuring service host...");
                
                composableServiceBoot.Open();

                Trace.WriteLine("OK");

                ThreadPool.QueueUserWorkItem(_ => {
                    Console.WriteLine("Press 'S' to shutdown server...");
                    while (Console.ReadKey(true).Key != ConsoleKey.S)
                        ;
                    // TODO: clean up running instances
                    messageQueue.PostQuit(0);
                });

                // Launch using default browser
                //System.Diagnostics.Process.Start(new Uri(BaseEndpoint, "restify/").ToString());
                // ...or click this from within Visual Studio: 
                //      http://localhost/restify/

                return messageQueue.RunMessageLoop();
            }
        }
    }
}
