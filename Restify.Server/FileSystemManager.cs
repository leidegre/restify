using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace Restify
{
    public class FileSystemManager
    {
        public static readonly FileSystemManager Current = new FileSystemManager();

        public bool FindFileName(string fileName, out string path)
        {
            var currentPath = Environment.CurrentDirectory;
            var currentRoot = Path.GetPathRoot(currentPath);
            var currentFile = Path.Combine(currentPath, fileName);

            while (currentFile.Length > currentRoot.Length && !File.Exists(currentFile))
                currentFile = Path.Combine((currentPath = Path.GetDirectoryName(currentPath)), fileName);

            if (File.Exists(currentFile))
            {
                path = currentFile;
                return true;
            }

            path = null;
            return false;
        }
    }
}
