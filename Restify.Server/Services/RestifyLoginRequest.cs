﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify.Services
{
    public class RestifyLoginRequest
    {
        public string UserName { get; set; }
        public string Password { get; set; }
    }
}