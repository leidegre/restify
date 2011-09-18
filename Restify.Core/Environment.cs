using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel.Composition;
using System.IO;
using System.Diagnostics.Contracts;

namespace Restify
{
    [Export(typeof(IEnvironment))]
    class Environment : IEnvironment
    {
        public VirtualPath GetVirtualPath(string path)
        {
            return new VirtualPath(Path.GetFullPath(path), false);
        }

        public VirtualPath GetVirtualPath(params string[] paths)
        {
            return new VirtualPath(Path.GetFullPath(Path.Combine(paths)), false);
        }

        public VirtualPath GetFile(VirtualPath virtualPath)
        {
            return new VirtualPath(virtualPath.Path, File.Exists(virtualPath.Path));
        }

        public VirtualPath GetDirectory(VirtualPath virtualPath)
        {
            if (!Directory.Exists(virtualPath.Path))
            {
                GetDirectory(new VirtualPath(Path.GetDirectoryName(virtualPath.Path), false));
                Directory.CreateDirectory(virtualPath.Path);
            }
            return new VirtualPath(virtualPath.Path, true);
        }

        public VirtualPath Find(VirtualPath virtualPath)
        {
            var find = virtualPath;
            while (!(find = GetFile(find)).IsValid)
            {
                var parent = FindParent(find);
                if (!(parent.Path.Length < find.Path.Length))
                    break;
                find = parent;
            }
            return find;
        }

        VirtualPath FindParent(VirtualPath virtualPath)
        {
            var fileName = Path.GetFileName(virtualPath.Path);
            var parentDirectoryName = Path.GetDirectoryName(Path.GetDirectoryName(virtualPath.Path));
            return new VirtualPath(Path.Combine(parentDirectoryName, fileName), false);
        }

        public IEnumerable<VirtualPath> GetFiles(VirtualPath virtualPath)
        {
            Contract.Requires(virtualPath.IsValid);
            return from file in Directory.GetFiles(virtualPath.Path) select new VirtualPath(file, true);
        }

        public string GetText(VirtualPath virtualPath)
        {
            Contract.Requires(virtualPath.IsValid);
            return File.ReadAllText(virtualPath.Path);
        }

        public byte[] GetBinary(VirtualPath virtualPath)
        {
            Contract.Requires(virtualPath.IsValid);
            return File.ReadAllBytes(virtualPath.Path);
        }

        public Stream GetStream(VirtualPath virtualPath)
        {
            Contract.Requires(virtualPath.IsValid);
            return new FileStream(virtualPath.Path, FileMode.Open, FileAccess.Read);
        }
    }
}
