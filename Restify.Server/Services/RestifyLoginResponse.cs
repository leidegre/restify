using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify.Services
{
    public class RestifyLoginResponse
    {
        public bool IsLoggedIn { get; set; }
        public string InstanceName { get; set; }
    }
}
