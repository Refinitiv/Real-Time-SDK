﻿/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    [Flags]
    public enum ServiceStateFlags : int
    {
        // (0x00) No flags set.
        NONE = 0x00,

        // (0x01) Indicates presence of the acceptingRequests member.
        HAS_ACCEPTING_REQS = 0x01,

        // (0x02) Indicates presence of the status member.
        HAS_STATUS = 0x02
    }
}
