using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel.Composition.Hosting;

namespace Restify.ComponentModel.Composition
{
    public interface IPartFactory
    {
        void Compose(CompositionContainer container);
    }
}
