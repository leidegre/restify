using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel;
using System.ServiceModel.Description;

namespace Testify
{
    class Program
    {
        static int Main(string[] args)
        {
            // *sign*
            
            var baseUri = "http://localhost:81/testify";

            if (args.Length > 0 && args[0] == "/Connect")
            {
                Connect(baseUri);
                return 0;
            }

            Console.WriteLine("Configuring service host...");

            var serviceHost = new ServiceHost(typeof(Service1));

            var endPoint = serviceHost.AddServiceEndpoint(typeof(IService1), new WebHttpBinding { }, baseUri);
            endPoint.Behaviors.Add(new WebHttpBehavior { FaultExceptionEnabled = true });

            var debugBehavior = serviceHost.Description.Behaviors.Find<ServiceDebugBehavior>();
            debugBehavior.IncludeExceptionDetailInFaults = true;

            serviceHost.Open();

            Console.WriteLine("OK");

            Console.WriteLine("Press the return key to continue...");
            Console.ReadLine();

            Console.WriteLine("Connecting to '{0}'...", baseUri);

            Connect(baseUri);

            //int exitCode;
            //using (var p = System.Diagnostics.Process.Start(typeof(Program).Assembly.Location, "/Connect"))
            //{
            //    p.WaitForExit();
            //    exitCode = p.ExitCode;
            //}

            //if (exitCode != 0)
            //    Console.WriteLine("Not OK {0}!", exitCode);
            //else
            //    Console.WriteLine("OK");

            Console.WriteLine("Press the return key to exit");
            Console.ReadLine();
            
            return 0;
        }

        private static void Connect(string baseUri)
        {
            var factory = new ChannelFactory<IService1>(new WebHttpBinding(), new EndpointAddress("http://127.0.0.1:8888/testify"));
            factory.Endpoint.Behaviors.Add(new WebHttpBehavior());

            var service = factory.CreateChannel();
            service.DoWork(new TestObject { TestValue = "testing..." });
        }
    }
}
