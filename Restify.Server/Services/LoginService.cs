using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.Text;
using System.Collections.Concurrent;
using System.Threading;
using System.Diagnostics;
using Restify.Threading;

namespace Restify.Services
{
    public class LoginService : ILoginService
    {
        public class SpotifyInstance : IDisposable
        {
            public string InstanceName { get; private set; }

            private Mutex instance;

            public SpotifyInstance()
            {
                var instanceName = Guid.NewGuid().ToString();

                var mutex = new CreateMutexMessage(string.Format("Global\\{0}Exit", instanceName));

                // if a mutex is acquired by a thread that later exits
                // the mutex will be abandoned, this is why we run a message loop
                // that way we can post and serialize actions on a single thread
                Program.PostSynchronized(mutex);

                InstanceName = instanceName;
                instance = mutex.Instance;
            }

            public void Shutdown()
            {
                Program.PostSynchronized(new ReleaseMutexMessage(instance));
                Process.WaitForExit();
            }

            public void Dispose()
            {
                instance.Dispose();
            }

            public Process Process { get; private set; }

            public void Start(ProcessStartInfo startInfo)
            {
                Process = Process.Start(startInfo);
            }

            private ChannelFactory<IBackEndService> channelFactory;

            public void Initialize()
            {
                channelFactory = new ChannelFactory<IBackEndService>(new BasicHttpBinding(), new EndpointAddress("http://localhost/restify/user/" + InstanceName));
            }

            public IBackEndService CreateClient()
            {
                //ICommunicationObject client;
                //try
                //{
                //    client = channelFactory.CreateChannel();
                //    // ...
                //    client.Close();
                //}
                //catch
                //{
                //    client.Abort();
                //}
                return channelFactory.CreateChannel();
            }
        }

        internal static Dictionary<string, SpotifyInstance> userMapping = new Dictionary<string, SpotifyInstance>(StringComparer.OrdinalIgnoreCase);

        private static SpotifyInstance CreateSpotifyClient(string userName)
        {
            Trace.WriteLine(string.Format("Creating new instance for user '{0}'", userName));

            var spotify = new SpotifyInstance();

            bool createdNew;
            using (var waitForInit = new Semaphore(0, 1, string.Format("Global\\{0}Init", spotify.InstanceName), out createdNew))
            {
                var currentExeFileName = Environment.GetCommandLineArgs()[0];

                var startInfo = new ProcessStartInfo();

                startInfo.CreateNoWindow = true;
                startInfo.UseShellExecute = false;
                startInfo.FileName = currentExeFileName;
                startInfo.Arguments = string.Format("/instanceName {0}", spotify.InstanceName);

                spotify.Start(startInfo);

                waitForInit.WaitOne();
            }

            spotify.Initialize();

            Trace.WriteLine(string.Format("Created instance '{0}' for user '{1}'", spotify.InstanceName, userName));

            return spotify;
        }

        public bool Login(RestifyLogin login)
        {
            IBackEndService svc;
            lock (userMapping)
            {
                SpotifyInstance backEndService;
                if (!userMapping.TryGetValue(login.UserName, out backEndService))
                {
                    userMapping.Add(login.UserName, backEndService = CreateSpotifyClient(login.UserName));
                }
                svc = backEndService.CreateClient();
            }
            try
            {
                var success = svc.Login(login);
                ((ICommunicationObject)svc).Close();
                return success;
            }
            catch (Exception)
            {
                ((ICommunicationObject)svc).Abort();
                throw;
            }
        }
    }
}
