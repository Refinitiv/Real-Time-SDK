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
    public enum ServiceLoadFlags : int
    {
        // (0x00) No flags set.
        NONE = 0x00,

        // (0x01) Indicates presence of an open limit on the service.
        HAS_OPEN_LIMIT = 0x01,

        // (0x02) Indicates presence of an open window on the service.
        HAS_OPEN_WINDOW = 0x02,

        // (0x04) Indicates presence of a load factor.
        HAS_LOAD_FACTOR = 0x04
    }
}
