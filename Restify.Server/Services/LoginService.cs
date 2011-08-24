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
using System.IO;

namespace Restify.Services
{
    public class LoginService : ILoginService
    {
        internal static Dictionary<string, SpotifyInstance> userMapping = new Dictionary<string, SpotifyInstance>(StringComparer.OrdinalIgnoreCase);

        private static SpotifyInstance CreateSpotifyClient(string userName)
        {
            Trace.WriteLine(string.Format("Creating new instance for user '{0}'", userName));

            var spotify = new SpotifyInstance(userName);

            bool createdNew;
            using (var waitForInit = new Semaphore(0, 1, string.Format(SpotifyInstance.GlobalBootInstanceFormatString, spotify.InstanceName), out createdNew))
            {
                var currentExeFileName = Environment.GetCommandLineArgs()[0];

                // libspotify needs it's own directory
                var workingDirectory = Path.Combine(Environment.CurrentDirectory, "user", userName);
                Directory.CreateDirectory(workingDirectory);

                var startInfo = new ProcessStartInfo();

                startInfo.CreateNoWindow = true;
                startInfo.UseShellExecute = false;
                startInfo.FileName = currentExeFileName;
                startInfo.Arguments = string.Format("/DllDirectory \"{0}\" /InstanceName {1}", Path.GetFullPath(Path.Combine(Environment.CurrentDirectory, @"..\..\..\lib")), spotify.InstanceName);
                startInfo.WorkingDirectory = workingDirectory;

                spotify.Start(startInfo);

                waitForInit.WaitOne();
            }

            spotify.Initialize();

            Trace.WriteLine(string.Format("Created instance '{0}' for user '{1}'", spotify.InstanceName, userName));

            return spotify;
        }

        private static bool HasInstance(string userName)
        {
            lock (userMapping)
            {
                return userMapping.ContainsKey(userName);
            }
        }

        private static SpotifyInstance GetInstance(string userName)
        {
            SpotifyInstance backEndService;
            lock (userMapping)
            {
                if (!userMapping.TryGetValue(userName, out backEndService))
                {
                    userMapping.Add(userName, backEndService = CreateSpotifyClient(userName));
                }
            }
            return backEndService;
        }

        // NOTE:
        //  To be able to issue a new request from here (which we do to talk to the external process)
        //  we need to actual do that in a different thread context (I don't know why) but if we don't
        //  WCF will generate all kinds of really strange exceptions
        //  All the ThreadContext class does is that it invokes the labmda/delegate/action 
        //  on a different thread and waits for the operation to complete before returning control to the caller

        public RestifyLoginResponse Query(RestifyLogin login)
        {
            if (HasInstance(login.UserName))
            {
                RestifyLoginResponse response = null;

                ThreadContext.Invoke(() => {
                    var spotify = GetInstance(login.UserName);
                    var proxy = spotify.CreateProxy();
                    using (new OperationContextScope((IContextChannel)proxy))
                    {
                        response = proxy.IsLoggedIn(login);
                    }
                });

                return response;
            }
            return new RestifyLoginResponse { IsLoggedIn = false };
        }

        public RestifyLoginResponse Login(RestifyLogin login)
        {
            RestifyLoginResponse response = null;
            
            ThreadContext.Invoke(() => {
                var spotify = GetInstance(login.UserName);
                var proxy = spotify.CreateProxy();
                using (new OperationContextScope((IContextChannel)proxy))
                {
                    response = proxy.Login(login);
                }
            });

            return response;
        }
    }
}
