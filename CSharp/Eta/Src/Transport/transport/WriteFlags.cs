/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Transports.Interfaces;

namespace Refinitiv.Eta.Transports
{
    /// <summary>
    /// ETA Write Flags passed into the
    /// <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/> method call.
    ///
    /// <seealso cref="IChannel"/>
    /// </summary>
    [Flags]
    public enum WriteFlags
    {

        /// <summary>
        /// No modification will be performed to this <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/> operation.
        /// </summary>
        NO_FLAGS = 0x00,

        /// <summary>
        /// Indicates that this message should not be compressed even if compression is enabled for this connection.
        /// </summary>
        DO_NOT_COMPRESS = 0x01,

        /// <summary>
        /// Write will attempt to pass the data directly to the transport, avoiding the queuing.
        /// If anything is currently queued, data will be queued.
        /// This option will increase CPU use but may decrease latency.
        /// </summary>
        DIRECT_SOCKET_WRITE = 0x02,

        /// <summary>
        /// Indicates that the writer wants to attach a sequence number to this message
        /// </summary>
        WRITE_SEQNUM = 0x04,

        /// <summary>
        /// Indicates that this message is a retransmission of previous content. 
        /// This requires a user supplied sequence number to indicate which packet is being retransmitted.
        /// </summary>
        WRITE_RETRANSMIT = 0x10,

        /// <summary>
        /// This is the max combined value of the bits mask that is allowed.
        /// </summary>
        MAX_VALUE = 0x10
    }
}
