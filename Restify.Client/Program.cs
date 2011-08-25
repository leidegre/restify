using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

namespace Restify
{
    class Client
    {
        [DllImport("kernel32.dll")]
        extern static bool SetDllDirectory(string lpPathName);

        static void Main(string[] args)
        {
            SetDllDirectory(Path.GetFullPath(@"..\..\..\lib"));
            Program.RunInstance("default");
        }
    }
}
