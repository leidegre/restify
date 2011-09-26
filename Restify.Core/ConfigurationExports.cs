using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Restify
{
    public static class ConfigurationExports
    {
        // AVOID CHANING THESE VALUES! 
        
        // They are complie-time constants, that means they are copied when the code is compiled.
        // This is different from how static members that are linked at run-time,
        // if you change any of these values, you need to recompile all assemblies that reference this one before it has any effect!

        public const string AgentId = "RESTify.AgentId";
        public const string BaseEndpoint = "RESTify.BaseEndpoint";
        public const string QueueEndpoint = "RESTify.QueueEndpoint";
    }
}
