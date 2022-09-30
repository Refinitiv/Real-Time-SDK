/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Example.Common
{
    public enum MarketPriceStatusFlags
    {
        // (0x00) No flags set.
        NONE = 0x00,

        // (0x01) Indicates presence of the state member.
        HAS_STATE = 0x01,

        // (0x02) Indicates presence of the private stream flag.
        PRIVATE_STREAM = 0x02
    }
}
