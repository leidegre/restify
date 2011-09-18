using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Restify.Client;
using System.IO;
using System.Threading;
using System.Collections.Concurrent;

namespace Restify
{
    public class SpotifyTestClient : IDisposable
    {
        static bool FindFileName(string fileName, out string path)
        {
            var currentPath = Environment.CurrentDirectory;
            var currentRoot = Path.GetPathRoot(currentPath);
            var currentFile = Path.Combine(currentPath, fileName);

            while (currentFile.Length > currentRoot.Length && !File.Exists(currentFile))
                currentFile = Path.Combine((currentPath = Path.GetDirectoryName(currentPath)), fileName);

            if (File.Exists(currentFile))
            {
                path = currentFile;
                return true;
            }

            path = null;
            return false;
        }

        private const int DefaultTimeout = 30 * 1000;

        private SpotifySession session;
        private Thread messageLoopThread;

        public SpotifyTestClient()
        {
            string appkey_path;
            if (!FindFileName("spotify_appkey.key", out appkey_path))
                throw new InvalidOperationException("Cannot find file 'spotify_appkey.key'.");
            
            session = new SpotifySession(new SpotifySessionConfiguration { ApplicationKey = File.ReadAllBytes(appkey_path) });
            session.MetadataUpdated += new Action(session_MetadataUpdated);
            
            messageLoopThread = new System.Threading.Thread(() => { session.RunMessageLoop(); });
            messageLoopThread.Start();
        }

        struct MetaobjectQuery
        {
            public ISpotifyMetaobject obj;
            public ManualResetEventSlim waitHandle;
        }

        ConcurrentBag<MetaobjectQuery> metaobjectQueue = new ConcurrentBag<MetaobjectQuery>();

        void session_MetadataUpdated()
        {
            session.Post(() => {

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
                        item.waitHandle.Set();
                    }
                }

                for (int i = 0; i < list.Count; i++)
                    metaobjectQueue.Add(list[i]);

            });
        }

        public void Login()
        {
            lock (session)
            {
                string spotify_user_path;
                if (!FindFileName("spotify_user", out spotify_user_path))
                    throw new InvalidOperationException("Cannot find file 'spotify_user'.");

                var credentials = File.ReadAllLines(spotify_user_path);
                if (!(credentials.Length > 1))
                    throw new InvalidOperationException("File 'spotify_user' does not a line for userName and password.");

                using (var waitHandle = new ManualResetEventSlim())
                {
                    SpotifyError err = SpotifyError.SP_ERROR_BAD_USERNAME_OR_PASSWORD;

                    Action<SpotifyError> loggedIn = e => {
                        err = e;
                        waitHandle.Set();
                    };

                    session.LoggedIn += loggedIn;
                    
                    session.Post(() => {
                        session.Login(credentials[0], credentials[1], false);
                    });

                    waitHandle.Wait(DefaultTimeout);
                    
                    session.LoggedIn -= loggedIn;

                    if (err != SpotifyError.SP_ERROR_OK)
                        throw new InvalidOperationException(string.Format("Cannot login using default credentials ({0}).", err));
                }
            }
        }

        public ISpotifyMetaobject GetMetaobject(string spotifyUri)
        {
            ISpotifyMetaobject metaobject = null;
            session.PostSynchronized(() => {
                metaobject = session.CreateMetaobject(spotifyUri);
            });
            if (metaobject != null && !metaobject.IsLoaded)
            {
                using (var waitHandle = new ManualResetEventSlim())
                {
                    metaobjectQueue.Add(new MetaobjectQuery { obj = metaobject, waitHandle = waitHandle });
                    if (!waitHandle.Wait(DefaultTimeout))
                        return null; // timeout
                }
            }
            return metaobject;
        }

        public void Dispose()
        {
            if (session != null)
            {
                session.StopMessageLoop();

                messageLoopThread.Join();

                session.Dispose();
                session = null;
            }
        }

        public void Play(SpotifyTrack track)
        {
            session.Post(() => {
                if (session.LoadTrack(track))
                    session.PlayTrack(true);
            });
        }

        public void Play(bool play)
        {
            session.Post(() => {
                session.PlayTrack(play);
            });
        }
    }
}
