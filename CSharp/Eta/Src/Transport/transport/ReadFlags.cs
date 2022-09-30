/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using Refinitiv.Eta.Transports.Interfaces;

namespace Refinitiv.Eta.Transports
{
    /// <summary>
    /// ETA Read Flags passed into the <see cref="IChannel.Read(ReadArgs, out Error)"/> method call.
    /// <seealso cref="IChannel"/>
    /// </summary>
    [Flags]
    public enum ReadFlags : int
    {
        /// <summary>
        /// No modification will be performed to this <see cref="IChannel.Read(ReadArgs, out Error)"/> operation.
        /// </summary>
        NO_FLAGS = 0x00,

        /// <summary>
        /// Set when valid node id is returned
        /// </summary>
        READ_NODE_ID = 0x02,

        /// <summary>
        /// Set when sequence number is returned
        /// </summary>
        READ_SEQNUM = 0x04,

        /// <summary>
        /// Set when the message has an instance ID set
        /// </summary>
        READ_INSTANCE_ID = 0x20,

        /// <summary>
        /// Indicates that this message is a retransmission of previous content
        /// </summary>
        READ_RETRANSMIT = 0x40
    }
}
