/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */


using Microsoft.IdentityModel.Tokens;

namespace LSEG.Ema.Access
{
    internal class HostPortParser
    {
        private readonly string defaultHost;
        private readonly string defaultPort;

        public HostPortParser(string defaultHost, string defaultPort) 
        {
            this.defaultHost = defaultHost;
            this.defaultPort = defaultPort;
        }

        public void StringToHostPort(string inputHost, out string host, out string port)
        {
            // Blank or null string indicates that this is the default defaultHost:defaultPort e.g. localhost:14002
            if (inputHost.IsNullOrEmpty() == true)
            {
                host = defaultHost;
                port = defaultPort;
                return;
            }

            int index = inputHost.IndexOf(":");

            // No ':' means it's just the hostname, so set default port.
            if (index == -1)
            {
                host = inputHost;
                port = defaultPort;
                return;
            }

            // If ':' is first, 
            if (index == 0)
            {
                host = defaultHost;
                port = inputHost.Substring(1);
                return;
            }

            string[] stringArray = inputHost.Split(':');

            // This covers the full "host:port" and "host:" strings.  Since we already know the 1st character isn't ':', the first substring will be a valid host name string.
            if (stringArray.Length == 2)
            {
                host = stringArray[0];
                if (stringArray[1].IsNullOrEmpty() == true)
                {
                    port = defaultPort;
                }
                else
                {
                    port = stringArray[1];
                }
                return;
            }
            else
            {
                throw new OmmInvalidConfigurationException("Host string is malformed. This should be [hostname]:[port].");
            }
        }
    }
}
