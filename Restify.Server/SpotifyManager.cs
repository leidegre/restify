using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using Restify.Client;
using System.IO;
using Restify.Services;

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
            const string spotify_appkey = "spotify_appkey.key";

            byte[] appkey = null;

#if DEBUG
            //
            // DEVELOPMENT NOTE:
            //
            //  If we find the file "spotify_appkey.key" any where up the directory tree
            //  we'll use that to initialize libspotify
            //
            //  The application key cannot be distributed and must be released as part of the
            //  application. It needs to be built into the manifest of the application before
            //  any bundles/downloads can be made (it should be relatively hidden as well, 
            //  non-trivial to get at it)
            //
            //  THIS FILE IS NOT SUPPOSED TO BE IN THE REPOSITORY
            //  AS IT CONTAINS SENSITIVE DATA!
            //

            var appkey_file = FindFile(spotify_appkey);
            if (!string.IsNullOrEmpty(appkey_file))
                appkey = File.ReadAllBytes(appkey_file);
#endif

            if (appkey == null)
            {
                throw new FileNotFoundException(string.Format("Cannot find Spotify application key file '{0}'.", spotify_appkey), spotify_appkey);
            }

            _session = new SpotifySession(appkey);
            _session.EndOfTrack += new Action(OnEndOfTrack);

            ThreadPool.QueueUserWorkItem(_ => _session.RunMessageLoop());
        }

        public static string FindFile(string fileName)
        {
            var currentPath = Environment.CurrentDirectory;
            var currentRoot = Path.GetPathRoot(currentPath);
            var currentFile = Path.Combine(currentPath, fileName);

            while (currentFile.Length > currentRoot.Length && !File.Exists(currentFile))
                currentFile = Path.Combine((currentPath = Path.GetDirectoryName(currentPath)), fileName);

            if (File.Exists(currentFile))
                return currentFile;

            return null;
        }

        public bool IsLoggedIn { get; private set; }

        public bool Login(string userName, string password)
        {
            lock (_session)
            {
                if (IsLoggedIn)
                    return true;

                SpotifyError error = SpotifyError.BadUsernameOrPassword;

                using (var wait = new ManualResetEventSlim())
                {
                    Action<SpotifyError> loggedIn = err => {
                        error = err;
                        wait.Set();
                    };

                    _session.LoggedIn += loggedIn;

                    _session.Post(() => {
                        _session.Login(userName, password, false);
                    });

                    if (!wait.Wait(_defaultTimeout))
                        throw new TimeoutException();

                    _session.LoggedIn -= loggedIn;
                }

                IsLoggedIn = error == SpotifyError.Ok;

                return IsLoggedIn;
            }
        }

        public List<SpotifyPlaylist> GetPlaylists()
        {
            return new List<SpotifyPlaylist>();
        }

        public SpotifyPlaylist GetPlaylist(string id)
        {
            return null;
        }

        public RestifySearchResult Search(string query)
        {
            SpotifySearchResult result = null;

            if (string.IsNullOrEmpty(query))
                return null;

            using (var wait = new ManualResetEventSlim())
            {
                // No need to unsubscribe here like we do with login because
                // the event is attached to this object which is cleaned up automatically
                // (the same thing is not possible with login, due to libspotify API design)

                var q = new SpotifySearchQuery { Query = query, TrackCount = 30 };

                q.Completed += r => {
                    result = r;
                    wait.Set();
                };

                _session.Post(() => {
                    _session.Search(q);
                });

                if (!wait.Wait(_defaultTimeout))
                    throw new TimeoutException();
            }

            var actualResult = new RestifySearchResult();

            if (result.Success)
            {
                var tracks = new List<RestifyTrack>();

                if (result.Tracks != null)
                {
                    foreach (var track in result.Tracks)
                    {
                        tracks.Add(new RestifyTrack { Id = track.Id, Title = track.Title, Artist = track.Artist, Length = RestifyTrack.ToString(track.Length) });
                    }
                }

                actualResult.Tracks = tracks;
            }

            return actualResult;
        }

        #region Playback

        RestifyTrack currentTrack;
        private bool _isPlaying;

        public void Play()
        {
            _session.Post(() => _session.PlayTrack(true));
            if (currentTrack == null)
            {
                OnEndOfTrack(); // grab a track
            }
        }

        public void PlayPause()
        {
            var play = !_isPlaying;
            _session.Post(() => _session.PlayTrack(play));
            _isPlaying = play;
        }

        public void Next()
        {
            OnEndOfTrack();
        }

        void OnEndOfTrack()
        {
            if (Program.InstanceName != null)
            {
                using (var client = new BackEndServiceClient(Program.BaseEndpoint + "/gateway"))
                {
                    var track = client.Dequeue();
                    
                    currentTrack = track;
                    
                    if (track != null)
                    {
                        _session.Post(() => {
                            var trackLink = new SpotifyLink(track.Id);
                            _session.UnloadTrack();
                            if (_session.LoadTrack(trackLink.CreateTrack()))
                            {
                                _session.PlayTrack(true);
                                _isPlaying = true;
                            }
                            else
                            {
                                currentTrack = null;
                                _isPlaying = false;
                            }
                        });
                    }
                }
            }
        }

        #endregion
    }
}
