using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify.Services
{
    public class RestifyPlaylistCollection
    {
        public bool IsLoaded { get; set; }
        public List<RestifyPlaylist> Playlists { get; set; }
    }
}
