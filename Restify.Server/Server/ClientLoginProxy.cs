using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Restify.ServiceModel;
using System.ServiceModel;

namespace Restify.Server
{
    class ClientLoginProxy : WebClient<ILoginService>, ILoginService
    {
        public ClientLoginProxy(Uri address)
            : base(address)
        {
        }

        public LoginResponse Login(LoginRequest login)
        {
            using (new OperationContextScope(InnerChannel))
            {
                return Channel.Login(login);
            }
        }
    }
}
