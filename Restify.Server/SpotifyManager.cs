﻿using System;
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
            const string spotify_appkey = "spotify_appkey.key";

            _session = new SpotifySession();

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

            _session.Initialize(appkey);

            ThreadPool.QueueUserWorkItem(_ => _session.Run());
        }

        public static string FindFile(string fileName)
        {
            var currentPath = Environment.CurrentDirectory;
            var currentRoot = Path.GetPathRoot(currentPath);
            var currentFile = Path.Combine(currentPath, fileName);

            while (currentFile.Length > currentPath.Length && !File.Exists(currentFile))
                currentFile = Path.Combine((currentPath = Path.GetDirectoryName(currentPath)), fileName);

            if (File.Exists(currentFile))
                return currentFile;
            
            return null;
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

        public SpotifyPlaylist GetPlaylist(string id)
        {
            var list = _session.GetPlaylistCollection();
            foreach (var pl in list)
            {
                if (pl.Id == id)
                    return pl;
            }
            return null;
        }

        public void Play(string id)
        {
            _session.PlayLink(id);
        }

        public void Enqueue(string id)
        {
            _session.EnqueueLink(id);
        }
    }
}
