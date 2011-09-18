using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Restify
{
    [TestClass]
    public class Ascii85Test
    {
        [TestMethod]
        public void TestMethod1()
        {
            var ascii85 = new Ascii85(new byte[] { 1, 2, 3 });
            var s = new Ascii85(ascii85.ToString());
            Assert.IsTrue(s.Equals(ascii85));
        }
    }
}
