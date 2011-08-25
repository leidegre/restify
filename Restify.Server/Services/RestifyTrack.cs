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
        public TimeSpan Length { get; set; }
    }
}
