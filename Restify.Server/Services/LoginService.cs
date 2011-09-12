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
        internal static Dictionary<string, SpotifyInstance> userInstanceMapping = new Dictionary<string, SpotifyInstance>(StringComparer.Ordinal);

        static LoginService()
        {
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
            string spotify_user;
            if (FileSystemManager.Current.FindFileName("spotify_user", out spotify_user))
            {
                var credentials = File.ReadAllLines(spotify_user);
                if (credentials.Length > 0)
                {
                    var instance = new SpotifyInstance(credentials[0], "default");
                    userMapping.Add(credentials[0], instance);
                    userInstanceMapping.Add(instance.InstanceName, instance);
                    if (credentials.Length > 1)
                    {
                        var client = instance.CreateClient();
                        client.Login(new RestifyLoginRequest { UserName = credentials[0], Password = credentials[1] });
                    }
                }
            }
#endif
        }

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
                    userInstanceMapping.Add(backEndService.InstanceName, backEndService);
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

        public RestifyLoginResponse Query(RestifyLoginRequest login)
        {
            if (HasInstance(login.UserName))
            {
                var spotify = GetInstance(login.UserName);
                var proxy = spotify.CreateClient();
                return proxy.IsLoggedIn(login);
            }
            return new RestifyLoginResponse { IsLoggedIn = false };
        }

        public RestifyLoginResponse Login(RestifyLoginRequest login)
        {
            var spotify = GetInstance(login.UserName);
            var client = spotify.CreateClient();
            return client.Login(login);
        }
    }
}
