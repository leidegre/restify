using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace Restify
{
    [DebuggerDisplay("{path, nq}, IsValid = {isValid}")]
    public struct VirtualPath
    {
        private string path;
        private bool isValid;

        public string Path { get { return path; } }
        public bool IsValid { get { return isValid; } }

        public VirtualPath(string path, bool isValid)
        {
            this.path = path;
            this.isValid = isValid;
        }

        public override string ToString()
        {
            return path;
        }
    }
}
