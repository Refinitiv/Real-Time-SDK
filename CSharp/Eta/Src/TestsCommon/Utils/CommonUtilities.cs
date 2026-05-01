/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Tests.Common.Utils
{
    public class CommonUtilities
    {
        /// <summary>
        /// Polls the given TCP port until it is free to bind, or the timeout expires.
        /// Fails the test if the port is still occupied when the deadline is reached.
        /// </summary>
        public static void WaitForPortAvailable(int port, TimeSpan timeout)
        {
            var deadline = System.DateTime.UtcNow + timeout;
            while (System.DateTime.UtcNow < deadline)
            {
                try
                {
                    using var listener = new System.Net.Sockets.TcpListener(System.Net.IPAddress.Loopback, port);
                    listener.Start();
                    listener.Stop();
                    return; // port is free
                }
                catch (System.Net.Sockets.SocketException)
                {
                    Thread.Sleep(500);
                }
            }
            Assert.Fail($"Port {port} is still in use after waiting {timeout.TotalSeconds} seconds.");
        }
    }
}

