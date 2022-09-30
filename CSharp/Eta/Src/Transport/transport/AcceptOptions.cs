/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Transports.Interfaces;

namespace Refinitiv.Eta.Transports
{
    /// <summary>
    /// ETA Accept Options used in the <see cref="IServer.Accept(AcceptOptions, out Error)"/> call.
    /// </summary>
    public class AcceptOptions
    {
        /// <summary>
        /// The default constructor to clear all options to default values.
        /// </summary>
        public AcceptOptions()
        {
            Clear();
        }

        /// <summary>
        /// Gets or sets whether the connection uses lock on read.
        /// </summary>
        /// <value><c>true</c> if the connection uses lock on read otherwise <c>false</c></value>
        public bool ChannelReadLocking { get; set; }

        /// <summary>
        /// Gets or sets whether the connection uses lock on write.
        /// </summary>
        /// <value><c>true</c> if the connection uses lock on write otherwise <c>false</c></value>
        public bool ChannelWriteLocking { get; set; }

        /// <summary>
        /// Gets or sets to indicate whether the server wants to reject the incoming
        /// connection. This may be due to some kind of connection limit being reached.
        /// For non-blocking connections to successfully complete rejection, the
        /// the initialization process must still be completed.
        /// </summary>
        public bool NakMount { get; set; }

        /// <summary>
        /// Gets or sets a user specified object. This value is not modified by the transport, but
        /// will be preserved and stored in the UserSpecObject of the <see cref="IChannel"/>
        /// returned from <see cref="IServer.Accept(AcceptOptions, out Error)"/>. If this value
        /// is not set, the <see cref="IChannel.UserSpecObject"/> will be set to the UserSpecObject
        /// associated with the Server that is accepting this connection.
        /// </summary>
        /// <value>The user specified object</value>
        public object UserSpecObject { get; set; }

        /// <summary>
        /// Gets or sets the size (in bytes) of the system's send buffer used for this connection,
        /// where applicable. Setting of 0 indicates to use default sizes.
        /// </summary>
        /// <value>The system's send buffer size</value>
        public int SysSendBufSize { get; set; }

        /// <summary>
        /// Gets or sets the size (in bytes) of the system's receive buffer used for this
        /// connection, where applicable. Setting of 0 indicates to use default
        /// sizes. Must be in the range of 0 - 2,147,483,647.
        /// </summary>
        /// <value>The system's receive buffer size</value>
        public int SysRecvBufSize { get; set; }

        /// <summary>
        /// Clears ETA Accept options.
        /// </summary>
        public void Clear()
        {
            ChannelReadLocking = false;
            ChannelWriteLocking = false;
            NakMount = false;
            UserSpecObject = null;
            SysSendBufSize = 0;
            SysRecvBufSize = 0;
        }
    }
}
