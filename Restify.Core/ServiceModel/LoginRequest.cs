using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify.ServiceModel
{
    public class LoginRequest
    {
        public string UserName { get; set; }
        public string Password { get; set; }
        public bool RememberMe { get; set; }
    }
}
