using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Linq;
using System.Text;
using Restify.ServiceModel;
using System.Diagnostics;
using System.Threading;
using System.Security.Cryptography;
using Restify.Client;

namespace Restify.Server
{
    using Item = KeyValuePair<bool, IClient>;

    [Export(typeof(IClientContainer))]
    class ClientContainer : IClientContainer
    {
        Dictionary<string, string> userMapping = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
        Dictionary<string, Item> uuidMapping = new Dictionary<string, Item>(StringComparer.Ordinal);

        class DebugClient : IClient
        {
            public string Id { get; set; }

            public void Initialize(string agentId)
            {
            }

            public object GetService(Type serviceType)
            {
                if (serviceType == typeof(ILoginService))
                {
                    return new ClientLoginProxy(new Uri(new Uri("http://localhost"), string.Format("restify/user/{0}/auth", Id)));
                }
                else if (serviceType == typeof(IClientService))
                {
                    return new BackEndServiceClient(new Uri(new Uri("http://localhost"), string.Format("restify/user/{0}", Id)));
                }
                return null;
            }
        }

        public IEnvironment Host { get; private set; }
        public ExportFactory<IClient> ClientFactory { get; private set; }

        [ImportingConstructor]
        public ClientContainer(IEnvironment host, ExportFactory<IClient> clientFactory)
        {
            this.Host = host;
            this.ClientFactory = clientFactory;
#if DEBUG
            //
            // DEVELOPMENT NOTE:
            //
            //  If we find the file "spotify_user" any where up the directory tree
            //  we'll try and create a default session using these credentials
            //   -  the first line in this file is the userName 
            //   -  the second line is the password (optional)
            //
            //  When this mode is enabled with a password it doesn't matter what you type in
            //  you'll already be logged in and subsequent atempts won't matter
            //
            //  THIS FILE IS NOT SUPPOSED TO BE IN THE REPOSITORY
            //  AS IT CONTAINS SENSITIVE DATA!
            //
            var user = Host.Find(Host.GetVirtualPath("spotify_user"));
            if (user.IsValid)
            {
                var credentials = Host.GetText(user).Split(new char[] { '\r', '\n' }, int.MaxValue, StringSplitOptions.RemoveEmptyEntries);
                if (credentials.Length > 0)
                {
                    var client = new DebugClient { Id = "default" };
                    userMapping[credentials[0]] = client.Id;
                    uuidMapping[client.Id] = new Item(false, client);
                    if (credentials.Length > 1)
                        using (var login = client.GetService<ILoginService>())
                            login.Login(new LoginRequest { UserName = credentials[0], Password = credentials[1] });
                }
            }
#endif
        }

        private IClient Create(string userName, string uuid)
        {
            var clientExport = ClientFactory.CreateExport();

            Trace.WriteLine(string.Format("Creating new instance for user '{0}'", userName));

            var client = clientExport.Value;

            client.Initialize(uuid);

            Trace.WriteLine(string.Format("New instance '{1}' for user '{0}' created", userName, uuid));

            userMapping.Add(userName, uuid);
            uuidMapping.Add(uuid, new Item(false, client));

            return client;
        }

        public IClient Create(string userName)
        {
            lock (this)
            {
                string uuid;
                if (userMapping.TryGetValue(userName, out uuid))
                    return uuidMapping[uuid].Value;

                using (var rng = new RNGCryptoServiceProvider())
                {
                    var uuidBytes = new byte[32];
                    rng.GetBytes(uuidBytes);
                    uuid = Base16.Encode(uuidBytes);
                }

                return Create(userName, uuid);
            }
        }

        public void Remove(IClient agent)
        {
            lock (this)
            {
                Item client;
                if (uuidMapping.TryGetValue(agent.Id, out client))
                {
                    uuidMapping.Remove(agent.Id);
                    var user = userMapping.FirstOrDefault(x => x.Value == agent.Id).Value;
                    userMapping.Remove(user);
                }
            }
        }

        public IClient GetPlayToken()
        {
            lock (this)
            {
                var item = uuidMapping.FirstOrDefault(x => x.Value.Key);
                if (!item.Value.Key)
                {
                    item = uuidMapping.FirstOrDefault();
                    if (item.Value.Value != null)
                        uuidMapping[item.Value.Value.Id] = new Item(true, item.Value.Value);
                }
                return item.Value.Value;
            }
        }

        public void Play()
        {
            var agent = GetPlayToken();
            using (var control = agent.GetService<IClientService>())
                control.Play();
        }

        public void PlayPause()
        {
            var agent = GetPlayToken();
            using (var control = agent.GetService<IClientService>())
                control.PlayPause();
        }

        public void Next()
        {
            var agent = GetPlayToken();
            using (var control = agent.GetService<IClientService>())
                control.Play();
        }
    }
}
