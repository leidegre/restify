using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify
{
    public interface IEnvironment
    {
        VirtualPath GetVirtualPath(string path);
        VirtualPath GetVirtualPath(params string[] paths);
        VirtualPath GetFile(VirtualPath virtualPath);
        VirtualPath GetDirectory(VirtualPath virtualPath);
        VirtualPath Find(VirtualPath virtualPath);
        IEnumerable<VirtualPath> GetFiles(VirtualPath virtualPath);
        string GetText(VirtualPath virtualPath);
        byte[] GetBinary(VirtualPath virtualPath);
        System.IO.Stream GetStream(VirtualPath virtualPath);
    }
}
