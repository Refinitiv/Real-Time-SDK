﻿/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Login Consumer Connection Status Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="LoginConsumerConnectionStatus"/>
    public enum LoginConsumerConnectionStatusFlags
    {
        /// <summary>(0x00) No flags set.</summary>
        NONE = 0x00,

        /// <summary>(0x01) Indicates presence of Warm Standby information.</summary>
        HAS_WARM_STANDBY_INFO = 0x01
    }
}
