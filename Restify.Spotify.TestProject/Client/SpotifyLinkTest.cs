using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Restify.Client
{
    [TestClass]
    public class SpotifyLinkTest
    {
        [TestMethod]
        public void ExternalTrackId()
        {
            using (var testClient = new SpotifyTestClient())
            {
                testClient.Login();
                var track = testClient.GetMetaobject("spotify:track:2qiKR1ojzjwyL4E41dXMsr") as SpotifyTrack;
            }
        }
    }
}
