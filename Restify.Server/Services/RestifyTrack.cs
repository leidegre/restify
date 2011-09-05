using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify.Services
{
    public class RestifyTrack
    {
        public string Id { get; set; }
        public string Title { get; set; }
        public string Artist { get; set; }
        public string Length { get; set; }

        public static string ToString(TimeSpan length)
        {
            return string.Format("{0:0}:{1:00}"
                , length.Ticks / TimeSpan.TicksPerMinute
                , (length.Ticks % TimeSpan.TicksPerMinute) / TimeSpan.TicksPerSecond
                );
        }
    }
}
