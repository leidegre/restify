using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify.ServiceModel.Composition
{
    public interface IComposableServiceMetadata
    {
        string Address { get; }
        string[] AddressMetadata { get; }
        Type ServiceType { get; }
    }
}
