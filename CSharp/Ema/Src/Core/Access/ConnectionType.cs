/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Connection types
    /// </summary>
    public enum ConnectionType
    {
        /// <summary>
		/// Unidentified connection type
		/// </summary>
        UNIDENTIFIED = -1,

        /// <summary>
		/// Indicates that the channel is using a standard TCP-based socket connection.
		/// </summary>
        SOCKET = 0,

        /// <summary>
        /// Indicates that the channel is using an SSL/TLS encrypted TCP-based socket connection.
        /// </summary>
        ENCRYPTED = 1
    }
}
