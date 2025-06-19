/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Protocol types.
    /// </summary>
    public enum ProtocolType
    {
        /// <summary>
        /// Unknown wire format protocol
        /// </summary>
        UNKNOWN = -1,

        /// <summary>
        /// Rssl wire format protocol
        /// </summary>
        RWF = 0,

        /// <summary>
        /// Rssl JSON protocol
        /// </summary>
        JSON = 2
    }
}
