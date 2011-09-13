using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Reflection;
using System.Diagnostics;
using System.IO;

namespace Restify
{
    class TestProgram
    {
        public static int Main(string[] args)
        {
            // This tool is used to run test in unique processes due to libspotify not being recyclable with-in the same process

            var q =
                from type in Assembly.Load("Restify.TestProject").GetExportedTypes()
                where type.GetCustomAttributes(typeof(TestClassAttribute), false).Length == 1
                select type
                ;

            var exitCode = 0;
            foreach (var testType in q)
            {
                var r = 
                    from method in testType.GetMethods()
                    where method.GetCustomAttributes(typeof(TestMethodAttribute), false).Length == 1
                    select method
                    ;

                foreach (var testMethod in r)
                {
                    var testStartInfo = new ProcessStartInfo {
                        FileName = @"C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE\MSTest.exe",
                        Arguments = string.Format("/nologo /testcontainer:\"{0}\" /test:{1}.{2} /noisolation", Path.GetFileName(testType.Assembly.Location), testType.FullName, testMethod.Name),
                        RedirectStandardOutput = true,
                        UseShellExecute = false,
                        CreateNoWindow = true,
                    };
                    using (var test = Process.Start(testStartInfo))
                    {
                        test.OutputDataReceived += (sender, e) => {
                            if (!string.IsNullOrEmpty(e.Data))
                                Console.WriteLine(e.Data);
                        };
                        test.BeginOutputReadLine();
                        test.WaitForExit();
                        if (test.ExitCode != 0)
                        {
                            Console.WriteLine("Test failed!");
                            exitCode = 1;
                        }
                        else
                            Console.WriteLine("Test succeeded");
                    }
                }
            }

            return exitCode;
        }
    }
}
