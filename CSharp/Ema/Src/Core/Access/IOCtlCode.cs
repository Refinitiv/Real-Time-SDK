/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
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
    /// <summary>
    /// Used for changing the max number of buffers.
    /// </summary>
    MAX_NUM_BUFFERS = 1,

    /// <summary>
    /// Used for changing the number of guaranteed buffers.
    /// </summary>
    NUM_GUARANTEED_BUFFERS = 2,

    /// <summary>
    /// Used to set the upper buffer usage threshold.
    /// </summary>
    HIGH_WATER_MARK = 3,

    /// <summary>
    /// Allows to change the TCP receive buffer size
    /// associated with the connection. Value is an int.
    /// </summary>
    /// <remarks>
    /// Please note that if the value is larger than 64K, the value needs to
    /// be specified before the socket is connected to the remote peer.
    /// </remarks>
    SYSTEM_READ_BUFFERS = 4,

    /// <summary>
    /// Allows to change the TCP send buffer size associated
    /// with the connection. Value is an int.
    /// </summary>
    SYSTEM_WRITE_BUFFERS = 5,

    /// <summary>
    /// Used to increase or decrease the number of server shared pool
    /// buffers. This option is used for IProvider applications only.
    /// </summary>
    SERVER_NUM_POOL_BUFFERS = 8,

    /// <summary>
    /// When compression is on, this value is the smallest size packet that will be
    /// compressed.
    /// </summary>
    COMPRESSION_THRESHOLD = 9
}
