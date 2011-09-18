using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.ServiceModel;
using System.Text;
using System.Threading;
using Restify.Threading;
using System.ServiceModel.Description;
using System.ServiceModel.Web;
using System.ServiceModel.Channels;
using Restify.Server;

namespace Restify.ServiceModel
{
    public class SpotifyInstance : IClient
    {
        public string UserName { get; private set; }
        public string InstanceName { get; private set; }
        
        public bool IsMaster { get; set; }

        private Mutex instance;

        public SpotifyInstance(string userName)
            : this(userName, Guid.NewGuid().ToString())
        {
        }

        public SpotifyInstance(string userName, string instanceName)
        {
            var mutex = new CreateMutexMessage(string.Format(Constant.GlobalWaitInstanceFormatString, instanceName));

            // if a mutex is acquired by a thread that later exits
            // the mutex will be abandoned, this is why we run a message loop
            // that way we can post and serialize actions on a single thread
            Program.PostSynchronized(mutex);

            UserName = userName;
            InstanceName = instanceName;
            instance = mutex.Instance;
        }

        public void Shutdown()
        {
            Program.PostSynchronized(new ReleaseMutexMessage(instance));
            var p = libspotify;
            if (p != null)
                p.WaitForExit();
        }

        public void Dispose()
        {
            instance.Dispose();
        }

        Process libspotify;

        public void Start(ProcessStartInfo startInfo)
        {
            libspotify = Process.Start(startInfo);
            libspotify.EnableRaisingEvents = true;
            libspotify.Exited += (sender, e) => {
                // Remember to clean up!
                Trace.WriteLine(string.Format("Instance '{0}' has exited", InstanceName));
                lock (LoginService.userMapping)
                {
                    LoginService.userMapping.Remove(UserName);
                }
                libspotify.Dispose();
                libspotify = null;
            };
        }


        private ChannelFactory<IClientService> clientFactory;

        public void Initialize()
        {
            //clientFactory = new ChannelFactory<IBackEndService>(new WebHttpBinding(), new EndpointAddress("http://localhost/restify/user/" + InstanceName));
            //clientFactory.Endpoint.Behaviors.Add(new WebHttpBehavior {
            //    AutomaticFormatSelectionEnabled = false,
            //    DefaultBodyStyle = WebMessageBodyStyle.Bare,
            //    DefaultOutgoingRequestFormat = WebMessageFormat.Json,
            //    DefaultOutgoingResponseFormat = WebMessageFormat.Json,
            //    FaultExceptionEnabled = true,
            //});

            //// step one - find and remove default endpoint behavior 
            //var defaultCredentials = channelFactory.Endpoint.Behaviors.Find<ClientCredentials>();
            //channelFactory.Endpoint.Behaviors.Remove(defaultCredentials);

            //// step two - instantiate your credentials
            //ClientCredentials loginCredentials = new ClientCredentials();
            //loginCredentials.UserName.UserName = "";
            //loginCredentials.UserName.Password = "";

            //// step three - set that as new endpoint behavior on factory
            //channelFactory.Endpoint.Behaviors.Add(loginCredentials); //add required ones
        }

        public BackEndServiceClient CreateClient()
        {
            return new BackEndServiceClient("http://localhost/restify/user/" + InstanceName);
        }
    }
}
