using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using Restify.ServiceModel.Composition;
using System.Diagnostics;
using System.Threading;
using Restify.Client;

namespace Restify
{
    class Program
    {
        [DllImport("kernel32.dll")]
        extern static IntPtr GetConsoleWindow();

        [DllImport("kernel32.dll")]
        extern static bool SetDllDirectory(string lpPathName);

        [Export(ConfigurationExports.AgentId)]
        public static string AgentId { get; private set; }

        [Export(ConfigurationExports.BaseEndpoint)]
        public static Uri BaseEndpoint { get; private set; }

        [Export(ConfigurationExports.QueueEndpoint)]
        public static Uri QueueEndpoint { get; private set; }

        static int Main(string[] args)
        {
            SetDllDirectory(Path.GetFullPath(@"..\..\..\lib"));

            if (GetConsoleWindow() != IntPtr.Zero)
                Trace.Listeners.Add(new ConsoleTraceListener());

            var it = ((IEnumerable<string>)args).GetEnumerator();
            while (it.MoveNext())
            {
                switch (it.Current)
                {
                    case "/agentId":
                        if (!it.MoveNext())
                            return Error("Option /agentId is missing argument string.");
                        AgentId = it.Current;
                        break;

                    case "/baseEndpoint":
                        if (!it.MoveNext())
                            return Error("Option /baseEndpoint is missing argument URI.");
                        BaseEndpoint = new Uri(it.Current);
                        break;

                    case "/queue":
                        if (!it.MoveNext())
                            return Error("Option /queue is missing argument URI.");
                        QueueEndpoint = new Uri(it.Current);
                        break;
                }
            }

            return RunAgent();
        }

        private static int Error(string error)
        {
            Trace.WriteLine(error, "Error");
            return 1;
        }

        public static int RunAgent()
        {
            var catalog = new AggregateCatalog(new AssemblyCatalog(typeof(Program).Assembly), new DirectoryCatalog(Environment.CurrentDirectory));

            var container = new CompositionContainer(catalog);

            var exitCode = 0;
            using (var composableServiceBoot = new ComposableServiceBoot(container, BaseEndpoint))
            {
                try
                {
                    composableServiceBoot.Open();
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
                    var s = string.Format(Constant.GlobalBootInstanceFormatString, AgentId);
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
                    var s = string.Format(Constant.GlobalWaitInstanceFormatString, AgentId);
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

                    var spotifyClient = container.GetExportedValue<ISpotifyClient>();
                    spotifyClient.Dispose();
                }
            }
            return exitCode;
        }
    }
}
