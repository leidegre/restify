using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using Restify.Client;
using System.IO;
using Restify.Services;
using System.Collections.Concurrent;

namespace Restify
{
    public sealed class SpotifyManager
    {
        public static SpotifyManager Current { get; private set; }

        private const int _defaultTimeout = 30 * 1000;

        static SpotifyManager()
        {
            Current = new SpotifyManager();
        }

        // We do no't expose the `SpotifySession` object directly
        // the manager needs to be thread safe because it's being invoked through
        // WCF service requests
        private SpotifySession _session;
        private Thread _sessionMessageLoopThread;

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

            string appkey_file;
            if (FileSystemManager.Current.FindFileName(spotify_appkey, out appkey_file))
                appkey = File.ReadAllBytes(appkey_file);
#endif

            if (appkey == null)
            {
                throw new FileNotFoundException(string.Format("Cannot find Spotify application key file '{0}'.", spotify_appkey), spotify_appkey);
            }

            _session = new SpotifySession(new SpotifySessionConfiguration { ApplicationKey = appkey });
            _session.MetadataUpdated += new Action(OnMetadataUpdated);
            _session.EndOfTrack += () => { OnEndOfTrack(false); };

            _sessionMessageLoopThread = new Thread(() => {
                _session.RunMessageLoop();
            });
            _sessionMessageLoopThread.Start();
        }


        struct MetaobjectQuery
        {
            public ISpotifyMetaobject obj;
            public ManualResetEventSlim evt;
        }

        private ConcurrentBag<MetaobjectQuery> metaobjectQueue = new ConcurrentBag<MetaobjectQuery>();

        void OnMetadataUpdated()
        {
            _session.Post(() => {

                var list = new List<MetaobjectQuery>();

                MetaobjectQuery item;
                while (metaobjectQueue.TryTake(out item))
                    list.Add(item);

                for (int i = 0; i < list.Count; i++)
                {
                    item = list[i];
                    item.obj.Update();
                    if (item.obj.IsLoaded)
                    {
                        list.RemoveAt(i--);
                        item.evt.Set();
                    }
                }

                for (int i = 0; i < list.Count; i++)
                    metaobjectQueue.Add(list[i]);

            });
        }

        public ISpotifyMetaobject CreateMetaobject(string spotifyUri)
        {
            ISpotifyMetaobject metaobject = null;
            _session.PostSynchronized(() => {
                metaobject = _session.CreateMetaobject(spotifyUri);
            });
            if (metaobject != null && !metaobject.IsLoaded)
            {
                using (var evt = new ManualResetEventSlim())
                {
                    metaobjectQueue.Add(new MetaobjectQuery { obj = metaobject, evt = evt });
                    if (!evt.Wait(_defaultTimeout))
                        return null; // timeout
                }
            }
            return metaobject;
        }

        public bool IsLoggedIn { get; private set; }

        public bool Login(string userName, string password)
        {
            lock (_session)
            {
                if (IsLoggedIn)
                    return true;

                SpotifyError error = SpotifyError.SP_ERROR_BAD_USERNAME_OR_PASSWORD;

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

                IsLoggedIn = error == SpotifyError.SP_ERROR_OK;

                return IsLoggedIn;
            }
        }

        ////public List<SpotifyPlaylist> GetPlaylists()
        //{
        //    return new List<SpotifyPlaylist>();
        //}

        //public SpotifyPlaylist GetPlaylist(string id)
        //{
        //    return null;
        //}

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
                OnEndOfTrack(false); // grab a track
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
            OnEndOfTrack(true);
        }

        void OnEndOfTrack(bool flush)
        {
            if (Program.InstanceName != null)
            {
                using (var client = new BackEndServiceClient(Program.BaseEndpoint + "/gateway"))
                {
                    var track = client.Dequeue();
                    
                    currentTrack = track;
                    
                    if (track == null)
                        return;

                    var trackMetaobject = (SpotifyTrack)CreateMetaobject(track.Id);
                    if (trackMetaobject == null)
                        return;

                    _session.Post(() => {
                        if (flush)
                            // this causes a discontinuity in the audio buffer 
                            // which will flush the audio buffer immediately
                            _session.Flush();
                        _session.UnloadTrack();
                        if (_session.LoadTrack(trackMetaobject))
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

        #endregion

        public void Shutdown()
        {
            if (_session != null)
            {
                _session.StopMessageLoop();

                if (_sessionMessageLoopThread != null)
                {
                    _sessionMessageLoopThread.Join();
                    _sessionMessageLoopThread = null;
                }

                _session.Dispose();
                _session = null;
            }
        }
    }
}
