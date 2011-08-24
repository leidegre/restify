using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using Restify.Client;
using System.IO;

namespace Restify
{
    public sealed class SpotifyManager
    {
        public static SpotifyManager Current { get; private set; }

        private static TimeSpan _defaultTimeout = new TimeSpan(30 * TimeSpan.TicksPerSecond);

        static SpotifyManager()
        {
            Current = new SpotifyManager();
        }

        // We do no't expose the `SpotifySession` object directly
        // the manager needs to be thread safe because it's being invoked through
        // WCF service requests
        SpotifySession _session;

        private SpotifyManager()
        {
            _session = new SpotifySession();

            byte[] appkey = null;

            if (File.Exists("spotify_appkey.key"))
                appkey = File.ReadAllBytes("spotify_appkey.key");
            else if (File.Exists(@"..\..\spotify_appkey.key"))
                appkey = File.ReadAllBytes(@"..\..\spotify_appkey.key");
            else if (File.Exists(@"..\..\..\..\spotify_appkey.key"))
                appkey = File.ReadAllBytes(@"..\..\..\..\spotify_appkey.key");

            if (appkey == null)
            {
                throw new InvalidOperationException("Cannot find spotify application key.");
            }

            _session.Initialize(appkey);

            ThreadPool.QueueUserWorkItem(_ => _session.Run());
        }

        public bool IsLoggedIn { get { return _session.IsLoggedIn; } }

        public bool Login(string user, string pass)
        {
            return _session.Login(user, pass);
        }

        public List<SpotifyPlaylist> GetPlaylists()
        {
            return _session.GetPlaylistCollection();
        }
    }
}
