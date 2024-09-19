/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// Return codes associated with the Transports namespace.
    /// </summary>
    public enum TransportReturnCode
    {
        /// <summary>
        /// Multicast Transport Specific Return Codes -70 through -61
        /// Transport Warning: Network congestion detected. Gaps are likely. */
        /// </summary>
        CONGESTION_DETECTED = -63,

        /// <summary>
        /// Transport Warning: Application is consuming more slowly than data
        /// is being provided. Gaps are likely.
        /// </summary>
        SLOW_READER = -62,

        /// <summary>
        /// Transport Warning: An unrecoverable packet gap was detected and some
        /// content may have been lost
        /// </summary>
        PACKET_GAP_DETECTED = -61,

        /* -35 through -60 reserved */

        /* -20 through -16 reserved */

        /// <summary>
        /// Transport Return Codes
        /// Because positive values indicate bytes left to read or write */
        /// Some negative transport layer return codes still indicate success */
        ///
        /// Transport Success: Indicates that a <see cref="IChannel.Read(ReadArgs, out Error)"/>
        /// call on the <see cref="IChannel"/> is already in progress. 
        /// This can be due to another thread performing the same operation.
        /// </summary>
        READ_IN_PROGRESS = -15,

        /// <summary>
        /// Transport Success: Indicates that the connections channel has changed.
        /// This can occur as a result of internal connection keep-alive mechanisms.
        /// </summary>
        READ_FD_CHANGE = -14,

       /// <summary>
       /// Transport Success: Indicates that a heartbeat message was received in
       /// Channel.read call. The ping timer should be updated.
       /// </summary>
        READ_PING = -13,

        /// <summary>
        /// Transport Success: Reading was blocked by the OS. Typically indicates
        /// that there are no bytes available to read, returned from 
        /// <see cref="IChannel.Read(ReadArgs, out Error)"/>
        /// </summary>
        READ_WOULD_BLOCK = -11,

        /// <summary>
        /// Transport Success:
        /// <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/>
        /// is fragmenting the buffer and needs to be called again with the same buffer. This
        /// indicates that Write was unable to send all fragments with the current
        /// call and must continue fragmenting.
        /// </summary>
        WRITE_CALL_AGAIN = -10,

        /// <summary>
        /// Transport Success:
        /// <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/> internally
        /// attempted to flush data to the connection but was blocked. This is not a
        /// failure and the user should not release their buffer.
        /// </summary>
        WRITE_FLUSH_FAILED = -9,

        /* -8 through -5 reserved */

        /// <summary>
        /// Transport Failure: There are no buffers available from the buffer pool,
        /// returned from <see cref="IChannel.GetBuffer(int, bool, out Error)"/>.
        /// </summary>
        NO_BUFFERS = -4,

        /// <summary>
        /// Transport Failure: Not initialized failure code, returned from transport
        /// methods when <see cref="Transport.Initialize(InitArgs, out Error)"/> did not succeed.
        /// </summary>
        INIT_NOT_INITIALIZED = -3,

        /// <summary>
        /// Transport Failure: Channel initialization failed/connection refused,
        /// returned from <see cref="IChannel.Init(InProgInfo, out Error)"/>
        /// </summary>
        CHAN_INIT_REFUSED = -2,

        /// <summary>
        /// General Failure and Success Codes
        /// General Failure: ETA general failure return code.
        /// </summary>
        FAILURE = -1,

        /// <summary>
        /// General Success: ETA general success return code.
        /// </summary>
        SUCCESS = 0,

        /// <summary>
        /// Transport Success: Channel initialization is In progress, returned from
        /// <see cref="IChannel.Init(InProgInfo, out Error)"/>
        /// </summary>
        CHAN_INIT_IN_PROGRESS = 2

        /* 3 through 9 reserved */
    }
}

