/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access;

/// <summary>
/// TransportProtocol represents transport protocol options
/// </summary>
public enum TransportProtocol
{
    /// <summary>
    /// Indicates undefined transport protocol
    /// </summary>
    UNKNOWN = 0,

    /// <summary>
    /// Indicates TCP transport protocol
    /// </summary>
    TCP = 1,

    /// <summary>
    /// Indicates Websocket transport protocol
    /// </summary>
    WEB_SOCKET = 2
}
