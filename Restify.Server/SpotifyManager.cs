using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using Restify.Spotify;

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
        Spotify.SpotifySession _session;

        private SpotifyManager()
        {
            _session = new Spotify.SpotifySession(null);
            _session.Initialize(Program.g_appkey);
            ThreadPool.QueueUserWorkItem(_ => _session.Run());
        }

        public bool IsLoggedIn { get { return _session.IsLoggedIn; } }

        public bool Login(string user, string pass)
        {
            lock (this)
            {
                using (var waitHandle = new ManualResetEventSlim(false))
                {
                    Spotify.SpotifyEventHandler loggedIn = null;
                    loggedIn =
                        (sender, e) => {
                            waitHandle.Set();
                            sender.LoggedIn -= loggedIn;
                        };
                    _session.LoggedIn += loggedIn;
                    _session.Login(user, pass);
                    waitHandle.Wait(_defaultTimeout);
                }
            }
            return IsLoggedIn;
        }

        public List<SpotifyPlaylist> GetPlaylists()
        {
            var playlists = new List<SpotifyPlaylist>();

            lock (this)
            {
                var count = _session.Playlists.Count;
                for (int i = 0; i < count; i++)
                {
                    var pl = _session.Playlists[i];
                    if (pl != null)
                    {
                        playlists.Add(pl);
                    }
                }
            }

            return playlists;
        }
    }
}
