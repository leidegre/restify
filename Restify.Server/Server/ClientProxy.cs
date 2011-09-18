using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel.Composition;
using System.Diagnostics;
using System.Threading;
using System.Security.Cryptography;
using Restify.Client;
using Restify.ServiceModel;

namespace Restify.Server
{
    [Export(typeof(IClient))]
    public class ClientProxy : IClient
    {
        public IEnvironment Host { get; private set; }
        public Uri BaseEndpoint { get; private set; }
        
        [ImportingConstructor]
        public ClientProxy(IEnvironment host, [Import(ConfigurationExports.BaseEndpoint)] Uri baseEndpoint)
        {
            this.Host = host;
            this.BaseEndpoint = baseEndpoint;
        }

        public string Id { get; private set; }

        private Process agent;

        public void Initialize(string agentId)
        {
            if (Id != null)
                throw new InvalidOperationException("Client has already been initialized.");

            bool createdNew;
            using (var waitForInit = new Semaphore(0, 1, string.Format(Constant.GlobalBootInstanceFormatString, agentId), out createdNew))
            {
                // each instance needs it's own directory
                var workingDirectory = Host.GetDirectory(Host.GetVirtualPath("user", agentId));

                var currentExeFileName = Host.GetVirtualPath(@"..\client\Restify.Agent.exe");

                var startInfo = new ProcessStartInfo();

                startInfo.CreateNoWindow = true;
                startInfo.UseShellExecute = false;
                startInfo.WorkingDirectory = workingDirectory.Path;
                startInfo.FileName = currentExeFileName.Path;
                startInfo.Arguments = string.Format("/agentId {0} /baseEndpoint \"{1}\" /queue \"{2}\"", agentId, BaseEndpoint, new Uri(BaseEndpoint, "/restify/master/queue"));

                agent = Process.Start(startInfo);
                agent.EnableRaisingEvents = true;
                agent.Exited += (sender, e) => {
                    Trace.WriteLine(string.Format("Instance '{0}' exited with exit code {1}", agentId, agent.ExitCode));
                    agent.Dispose();
                    agent = null;
                    // TODO: clean up "the element is still in the IClientContainer"
                };

                waitForInit.WaitOne();
            }

            this.Id = agentId;
        }

        public object GetService(Type serviceType)
        {
            if (serviceType == typeof(ILoginService))
            {
                return new ClientLoginProxy(new Uri(BaseEndpoint, string.Format("restify/user/{0}/auth", Id)));
            }
            else if (serviceType == typeof(IClientService))
            {
                return new BackEndServiceClient(new Uri(BaseEndpoint, string.Format("restify/user/{0}", Id)));
            }
            return null;
        }
    }
}
