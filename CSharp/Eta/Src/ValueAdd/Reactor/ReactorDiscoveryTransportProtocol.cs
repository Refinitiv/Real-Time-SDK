/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Types indicating the transport query parameter.
    /// </summary>
    /// <seealso cref="ReactorServiceDiscoveryOptions"/>
    public enum ReactorDiscoveryTransportProtocol : int
    {
        /// <summary>
        /// Unknown transport protocol
        /// </summary>
        RD_TP_INIT = 0,

        /// <summary>
        /// TCP transport protocol
        /// </summary>
        RD_TP_TCP = 1,

        /// <summary>
        /// Websocket transport protocol
        /// </summary>
        RD_TP_WEBSOCKET = 2
    }
}
