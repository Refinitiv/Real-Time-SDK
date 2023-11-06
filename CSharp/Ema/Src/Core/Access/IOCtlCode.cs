/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access;


/// <summary>
/// IOCtlCode class provides I/O codes for modifying I/O values programmatically using the
/// ModifyIOCtl() method of provided by OmmConsumer class.
/// </summary>
/// <remarks>
/// <para>
/// IOCtlCode defines numeric representation of I/O codes to modify option for a
/// particular channel or server.</para>
///
/// Code snippet:
/// <pre>
///  // This consumer variable is created for consumer applications.
/// OmmConsumer consumer;
///
///  // Modifies the number of guaranteed buffers for the underlying channel.
/// consumer.ModifyIOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, 500);
/// </pre>
/// </remarks>
public enum IOCtlCode : int
{
    /// Used for changing the max number of buffers.
    MAX_NUM_BUFFERS = 1,

    /// Used for changing the number of guaranteed buffers.
    NUM_GUARANTEED_BUFFERS = 2,

    /// Used to set the upper buffer usage threshold.
    HIGH_WATER_MARK = 3,

    /// Allows to change the TCP receive buffer size
    /// associated with the connection. Value is an int.
    /// <br/>
    /// Please note that if the value is larger than 64K, the value needs to
    /// be specified before the socket is connected to the remote peer.
    SYSTEM_READ_BUFFERS = 4,

    /// Allows to change the TCP send buffer size associated
    /// with the connection. Value is an int.
    SYSTEM_WRITE_BUFFERS = 5,

    /// When compression is on, this value is the smallest size packet that will be
    /// compressed.
    COMPRESSION_THRESHOLD = 9
}
