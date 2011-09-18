using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Restify.Client
{
    [TestClass]
    public class SpotifyMusicTest
    {
        [TestMethod]
        public void MusicTest()
        {
            using (var testClient = new SpotifyTestClient())
            {
                testClient.Login();
                var track = testClient.GetMetaobject("spotify:track:2qiKR1ojzjwyL4E41dXMsr") as SpotifyTrack;
                testClient.Play(track);
                System.Threading.Thread.Sleep(10 * 1000);
            }
        }

        [TestMethod]
        public void MusicPlayPauseTest()
        {
            using (var testClient = new SpotifyTestClient())
            {
                testClient.Login();
                var track = testClient.GetMetaobject("spotify:track:6TbYnMMWfdUZXQm6tZ6vib") as SpotifyTrack;
                testClient.Play(track);
                System.Threading.Thread.Sleep(1 * 1000);
                testClient.Play(false);
                System.Threading.Thread.Sleep(1 * 1000);
                testClient.Play(true);
                System.Threading.Thread.Sleep(1 * 1000);
            }
        }
    }
}
