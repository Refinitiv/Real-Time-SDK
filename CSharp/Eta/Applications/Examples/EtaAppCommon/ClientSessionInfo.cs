/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// Client session information.
    /// </summary>
    public class ClientSessionInfo
    {
        internal PingHandler PingHandler { get; set; } = new PingHandler();

        public IChannel? ClientChannel { get; internal set; }

        public long StartTime { get; internal set; }

        public SelectElement SelectElement { get; private set; } = new SelectElement();

        /// <summary>
        /// Default constructor
        /// </summary>
        public ClientSessionInfo()
        {
            SelectElement.UserSpec = this;
        }

        /// <summary>
        /// Clears to default values
        /// </summary>
        public void Clear()
        {
            ClientChannel = null;
            PingHandler.Clear();
            StartTime = 0;
            SelectElement.Clear();
        }
    }
}
