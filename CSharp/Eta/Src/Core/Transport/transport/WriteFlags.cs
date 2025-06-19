/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using LSEG.Eta.Common;

namespace LSEG.Eta.Transports
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
        /// This is the max combined value of the bits mask that is allowed.
        /// </summary>
        MAX_VALUE = DO_NOT_COMPRESS | DIRECT_SOCKET_WRITE
    }
}
