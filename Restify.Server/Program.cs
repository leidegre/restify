using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.ServiceModel;
using System.ServiceModel.Description;
using System.Text;
using System.Threading;
using Restify.Services;
using Restify.Threading;
using System.ServiceModel.Web;
using System.ServiceModel.Dispatcher;
using System.ServiceModel.Channels;

namespace Restify
{
    public class Program
    {
        [DllImport("kernel32.dll")]
        extern static IntPtr GetConsoleWindow();

        [DllImport("kernel32.dll")]
        extern static bool SetDllDirectory(string lpPathName);

        static int Main(string[] args)
        {
            // This call cannot be made in a method that uses
            // any reference to the "Restify.Spotify" wrapper
            // it will trigger a load of the "libspotify.dll" 
            // which if not found will crash the app
            // "libspotify.dll" has to be found in the DLL search paths
            SetDllDirectory(Path.GetFullPath(@"..\..\..\lib"));

            // If launched with a console window 
            // (this is not the case when running as a service or 
            // when creating child processes)
            if (GetConsoleWindow() != IntPtr.Zero)
                Trace.Listeners.Add(new ConsoleTraceListener());

            Trace.WriteLine(string.Format("CurrentDirectory: {0}", Environment.CurrentDirectory));

            string instanceName = null;

            for (int i = 0; i < args.Length; i++)
            {
                if ("/DllDirectory".Equals(args[i], StringComparison.OrdinalIgnoreCase))
                {
                    if (!(i + 1 < args.Length))
                    {
                        Trace.WriteLine("Option /DllDirectory missing argument: path", "Error");
                        return 1;
                    }
                    var dllDirectory = Path.GetFullPath(args[++i]);
                    if (!SetDllDirectory(dllDirectory))
                    {
                        Trace.WriteLine(string.Format("Option /DllDirectory: cannot set DLL directory to '{0}'", dllDirectory), "Error");
                        return 1;
                    }
                }
                else if ("/InstanceName".Equals(args[i], StringComparison.OrdinalIgnoreCase))
                {
                    if (!(i + 1 < args.Length))
                    {
                        Trace.WriteLine("Option /InstanceName missing argument: instanceName", "Error");
                        return 1;
                    }
                    var s = args[++i];
                    if (string.IsNullOrEmpty(s) || s.IndexOfAny(Path.GetInvalidPathChars()) != -1)
                    {
                        Trace.WriteLine(string.Format("Option /InstanceName argument instanceName '{0}' is invalid", s), "Error");
                        return 1;
                    }
                    instanceName = s;
                }
            }

            if (string.IsNullOrEmpty(instanceName))
            {
                // run front end and gateway
                return RunServer();
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

        class AclDispatchMessageInspector : IDispatchMessageInspector
        {
            public object AfterReceiveRequest(ref System.ServiceModel.Channels.Message request, IClientChannel channel, InstanceContext instanceContext)
            {
                var instanceName = WebOperationContext.Current.IncomingRequest.Headers["X-RESTify-Instance"];
                lock (LoginService.userMapping)
                {
                    if (!LoginService.userInstanceMapping.ContainsKey(instanceName))
                    {
                        throw new InvalidOperationException();
                    }
                }
                return null;
            }

            public void BeforeSendReply(ref System.ServiceModel.Channels.Message reply, object correlationState)
            {
            }
        }

        class AclEndpointBehavior : IEndpointBehavior
        {
            public void AddBindingParameters(ServiceEndpoint endpoint, System.ServiceModel.Channels.BindingParameterCollection bindingParameters)
            {
            }

            public void ApplyClientBehavior(ServiceEndpoint endpoint, System.ServiceModel.Dispatcher.ClientRuntime clientRuntime)
            {
            }

            public void ApplyDispatchBehavior(ServiceEndpoint endpoint, System.ServiceModel.Dispatcher.EndpointDispatcher endpointDispatcher)
            {
                endpointDispatcher.DispatchRuntime.MessageInspectors.Add(new AclDispatchMessageInspector());
            }

            public void Validate(ServiceEndpoint endpoint)
            {
            }
        }

        public static int RunServer()
        {
            MessageQueue = new MessageQueue();

            Trace.WriteLine("Configuring service host...");

            var frontEndHost = new ServiceHost(typeof(FrontEndService));
            var frontEndBaseUri = "http://localhost/restify";
            var frontEndEndPoint = frontEndHost.AddServiceEndpoint(typeof(IFrontEndService), new WebHttpBinding(), frontEndBaseUri);
            frontEndEndPoint.Behaviors.Add(new WebHttpBehavior {
                AutomaticFormatSelectionEnabled = false,
                DefaultBodyStyle = WebMessageBodyStyle.Bare,
                DefaultOutgoingRequestFormat = WebMessageFormat.Json,
                DefaultOutgoingResponseFormat = WebMessageFormat.Json,
                FaultExceptionEnabled = true,
                HelpEnabled = true,
            });
            frontEndHost.Open();
            
            Trace.WriteLine(frontEndBaseUri);

            var loginHost = new ServiceHost(typeof(LoginService));
            var loginBaseUri = "http://localhost/restify/auth";
            var loginEndPoint = loginHost.AddServiceEndpoint(typeof(ILoginService), new WebHttpBinding(), loginBaseUri);
            loginEndPoint.Behaviors.Add(new WebHttpBehavior {
                AutomaticFormatSelectionEnabled = false,
                DefaultBodyStyle = WebMessageBodyStyle.Bare,
                DefaultOutgoingRequestFormat = WebMessageFormat.Json,
                DefaultOutgoingResponseFormat = WebMessageFormat.Json,
                FaultExceptionEnabled = true,
                HelpEnabled = true,
            });
            loginHost.Open();
            
            Trace.WriteLine(loginBaseUri);

            var gatewayHost = new ServiceHost(typeof(BackEndGatewayService));
            var gatewayBaseUri = "http://localhost/restify/gateway";
            var gatewayEndPoint = gatewayHost.AddServiceEndpoint(typeof(IBackEndService), new WebHttpBinding(), gatewayBaseUri);
            gatewayEndPoint.Behaviors.Add(new WebHttpBehavior {
                AutomaticFormatSelectionEnabled = false,
                DefaultBodyStyle= WebMessageBodyStyle.Bare,
                DefaultOutgoingRequestFormat = WebMessageFormat.Json,
                DefaultOutgoingResponseFormat = WebMessageFormat.Json,
                FaultExceptionEnabled = true,
                HelpEnabled = true,
            });
            //gatewayEndPoint.Behaviors.Add(new AclEndpointBehavior());
            gatewayHost.Open();

            Trace.WriteLine(gatewayBaseUri);

            //WebOperationContext.Current.IncomingRequest.Headers[GatewayHeaderName];

            Trace.WriteLine("OK");

            ThreadPool.QueueUserWorkItem(_ => {
                Console.WriteLine("Press 'S' to shutdown server...");
                while (Console.ReadKey(true).Key != ConsoleKey.S)
                    ;
                lock (LoginService.userMapping)
                {
                    // .ToList() prevents collection was modified during enumeration error
                    // from being thrown
                    foreach (var item in LoginService.userMapping.ToList()) 
                    {
                        item.Value.Shutdown();
                    }
                }
                MessageQueue.PostQuit(0);
            });

            System.Diagnostics.Process.Start("http://localhost/restify");

            return MessageQueue.RunMessageLoop();
        }

        public static string InstanceName { get; private set; }

        public static int RunInstance(string instanceName)
        {
            var exitCode = 0;
            using (var serviceHost = new ServiceHost(typeof(BackEndService)))
            {
                try
                {
                    InstanceName = instanceName;

                    var baseUri = "http://localhost/restify/user/" + instanceName;

                    var endPoint = serviceHost.AddServiceEndpoint(typeof(IBackEndService), new WebHttpBinding { }, baseUri);
                    
                    endPoint.Behaviors.Add(new WebHttpBehavior {
                        DefaultBodyStyle = System.ServiceModel.Web.WebMessageBodyStyle.Bare,
                        DefaultOutgoingRequestFormat = System.ServiceModel.Web.WebMessageFormat.Json,
                        DefaultOutgoingResponseFormat = System.ServiceModel.Web.WebMessageFormat.Json,
                        FaultExceptionEnabled = true,
                        HelpEnabled = true,
                    });

                    //var metadataBehavior = serviceHost.Description.Behaviors.Find<ServiceMetadataBehavior>();
                    //metadataBehavior.HttpGetEnabled = true;

                    //serviceHost.Description.Behaviors.Add(new ServiceMetadataBehavior {
                    //    HttpGetUrl = new Uri(baseUri),
                    //    HttpGetEnabled = true
                    //});

                    var debugBehavior = serviceHost.Description.Behaviors.Find<ServiceDebugBehavior>();
                    debugBehavior.IncludeExceptionDetailInFaults = true;
                    
                    serviceHost.Open();

                    Trace.WriteLine(endPoint.Address);
                }
                catch (Exception ex)
                {
                    if (Debugger.IsAttached)
                        Debugger.Break();

                    Trace.WriteLine(string.Format("{0}: {1}", ex.GetType(), ex.Message), "Error");
                    exitCode = 1;
                }
                finally
                {
                    // Signal that the instance is running
                    var s = string.Format(SpotifyInstance.GlobalBootInstanceFormatString, instanceName);
                    bool createdNew;
                    using (var waitForInit = new Semaphore(0, 1, s, out createdNew))
                    {
                        if (!createdNew)
                            waitForInit.Release();
                    }
                }

                if (exitCode == 0)
                {
                    // Block thread, either by an existing system-wide Mutex
                    // or wait for a key press
                    var s = string.Format(SpotifyInstance.GlobalWaitInstanceFormatString, instanceName);
                    bool createdNew;
                    using (var waitForExit = new Mutex(false, s, out createdNew))
                    {
                        if (!createdNew)
                            try
                            {
                                waitForExit.WaitOne();
                            }
                            catch (AbandonedMutexException)
                            {
                                // this should only happen if the parent process is killed (or crashed)
                                // as long as the original thread shutdown gracefully, this shouldn't occur
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
