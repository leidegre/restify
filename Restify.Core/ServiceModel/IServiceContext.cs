using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify.ServiceModel
{
    public interface IServiceContext
    {
        string GetParameter(string parameterName);
    }
}
