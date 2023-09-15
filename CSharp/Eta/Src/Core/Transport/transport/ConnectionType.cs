/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// ETA Connection types are used in several areas of the transport. When
    /// creating a connection an application can specify the connection type to use.
    /// </summary>
    public enum ConnectionType
    {
        /// <summary>
        /// Indicates that the <see cref="IChannel"/> is using a standard TCP-based socket
        /// connection. This type can be used to connect between any ETA Transport
        /// based applications.
        /// </summary>
        SOCKET = 0,

        /// <summary>
        /// Indicates that the <see cref="IChannel"/> is using an SSL/TLS encrypted
        /// TCP-based socket connection. This type can be used by
        /// a ETA Transport consumer based application.
        /// </summary>
        ENCRYPTED = 1,

        /// <summary>
        /// max defined connectionType
        /// </summary>
        MAX_DEFINED = ENCRYPTED
    }
}
