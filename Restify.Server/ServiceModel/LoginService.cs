﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.Text;
using System.Collections.Concurrent;
using System.Threading;
using System.Diagnostics;
using Restify.Threading;
using System.IO;
using Restify.ServiceModel;
using Restify.ServiceModel.Composition;
using Restify.Server;
using System.ComponentModel.Composition;
using Restify.Client;

namespace Restify.ServiceModel
{
    [ComposableServiceExport("/restify/auth", typeof(LoginService))]
    public class LoginService : ILoginService
    {
        private IClientContainer container;
        private IServiceContext context;

        [ImportingConstructor]
        public LoginService(IClientContainer clientContainer, [Import(AllowDefault = true)] IServiceContext context)
        {
            this.container = clientContainer;
            this.context = context;
        }

        public string Ping()
        {
            return context.GetParameter("X-RESTify-Instance");
        }

        public LoginResponse Login(LoginRequest login)
        {
            var client = container.Create(login.UserName);
            using (var loginService = client.GetService<ILoginService>())
            {
                var response = loginService.Login(login);
                if (response.LoginError == SpotifyClientLoginError.LoggedIn)
                    response.InstanceName = client.Id; // return the client session id, if the authentication was successful
                return response;
            }
        }

        void IDisposable.Dispose()
        {
        }
    }
}
