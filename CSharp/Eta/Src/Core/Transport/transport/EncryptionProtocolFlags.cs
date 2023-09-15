/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// Encryption protocol types specified with the <see cref="ConnectionType.ENCRYPTED"/> connection type.
    /// </summary>
    [Flags]
    public enum EncryptionProtocolFlags
    {
        /// <summary>
        /// Allows the operating system to choose the best protocol to use, and to block protocols that are not secure. 
        /// Unless your app has a specific reason not to, you should use this field
        /// </summary>
        ENC_NONE = 0x00,

        /// <summary>
        /// Encryption using TLSv1.2 protocol
        /// </summary>
        ENC_TLSV1_2 = 0x04
    }
}
