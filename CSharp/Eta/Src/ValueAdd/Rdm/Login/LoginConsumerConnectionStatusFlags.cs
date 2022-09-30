/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// RDM Login Consumer Connection Status flags.
    /// </summary>
    public enum LoginConsumerConnectionStatusFlags
    {
        /// <summary>(0x00) No flags set.</summary>
        NONE = 0x00,

        /// <summary>(0x01) Indicates presence of Warm Standby information.</summary>
        HAS_WARM_STANDBY_INFO = 0x01
    }
}
