using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify.Client
{
    public enum SpotifyClientLoginError
    {
        BadUserNamePassword = 0,
        LoggedIn = 1,
        UserNeedsPremium = 2,
    }
}
