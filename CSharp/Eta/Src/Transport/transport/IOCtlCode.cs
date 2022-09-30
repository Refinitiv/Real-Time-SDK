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
    /// ETA IOCtl codes for <see cref="IChannel.IOCtl(IOCtlCode, int, out Error)"/>, 
    /// <see cref="IChannel.IOCtl(IOCtlCode, object, out Error)"/> and <see cref="IServer.IOCtl(IOCtlCode, int, out Error)"/>
    /// </summary>
    public enum IOCtlCode
    {
        /// <summary>
        /// Allows a <see cref="IChannel"/> to change its MaxOutputBuffers setting.
        /// Default is 50.
        /// <see cref="BindOptions.MaxOutputBuffers"/> for more information.
        /// </summary>
        MAX_NUM_BUFFERS = 1,

        /// <summary>
        /// Allows a <see cref="IChannel"/> to change its GuaranteedOutputBuffers setting.
        /// Default is 50.
        /// <see cref="BindOptions.GuaranteedOutputBuffers"/> or <see cref="ConnectOptions.GuaranteedOutputBuffers"/>
        /// for more information.
        /// </summary>
        NUM_GUARANTEED_BUFFERS = 2,

        /// <summary>
        /// Allows a <see cref="IChannel"/> to change the internal ETA output queue depth
        /// water mark, which has a default value of 6,144 bytes. When the ETA output
        /// queue exceeds this number of bytes, the write method internally attempts
        /// to flush content to the network.
        /// Default is 6144.
        /// </summary>
        HIGH_WATER_MARK = 3,

        /// <summary>
        /// Allows a <see cref="IChannel"/> to change the TCP receive buffer size associated
        /// with the connection. Default is 64K.
        /// </summary>
        /// <remarks>
        /// Please note that if the value is larger than 64K, the value needs to be specified before
        /// the socket is connected to the remote peer.
        /// For servers and <c>SYSTEM_READ_BUFFERS</c> larger than 64K, use <see cref="BindOptions.SysRecvBufSize"/>
        /// to set the receive buffer size, prior to calling <see cref="Transport.Bind(BindOptions, out Error)"/>.
        /// For clients and <c>SYSTEM_READ_BUFFERS</c> larger than 64K, use <see cref="ConnectOptions.SysRecvBufSize"/>
        /// to set the receive buffer size, prior to calling <see cref="Transport.Connect(ConnectOptions, out Error)"/>.
        /// </remarks>
        SYSTEM_READ_BUFFERS = 4,

        /// <summary>
        /// Allows a <see cref="IChannel"/> to change the TCP send buffer size associated
        /// with the connection. Default is 64K.
        /// </summary>
        SYSTEM_WRITE_BUFFERS = 5,

        /// <summary>
        /// Allows a <see cref="IChannel"/> to change its PriorityFlushStrategy. Value is a <c>string</c>,
        /// where each entry in the <c>string</c> is either:
        /// H for high priority
        /// M for medium priority
        /// L for low priority
        /// The String should not exceed 32 entries. At least one H and one M must be present, however no L is required.
        /// If no low priority flushing is specified, the low priority queue will only be flushed when no other data
        /// is available for output. Default is "HMHLHM".
        /// </summary>
        PRIORITY_FLUSH_ORDER = 7,

        /// <summary>
        /// Allows a <see cref="IServer"/> to change its SharedPoolSize setting.
        /// </summary>
        SERVER_NUM_POOL_BUFFERS = 8,

        /// <summary>
        /// Allows a <see cref="IChannel"/> to change the size (in bytes) at which buffer compression will occur,
        /// must be greater than 30 bytes and 300 bytes for <see cref="CompressionType.ZLIB"/> and <see cref="CompressionType.LZ4"/>
        /// respectively. Default is 30 bytes for <see cref="CompressionType.ZLIB"/> and 300 bytes for <see cref="CompressionType.LZ4"/>
        /// </summary>
        COMPRESSION_THRESHOLD = 9,

        /// <summary>
        /// Allows a <see cref="IServer"/> to reset the PeakBufferUsage statistic.
        /// </summary>
        /// <remarks>
        /// Value is not required.
        /// </remarks>
        SERVER_PEAK_BUF_RESET = 10
    }
}
