/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Example.Common
{
    [Flags]
    public enum ItemDomainCommonFlags
    {
        // (0x00) No flags set.
        NONE = 0x00,

        // (0x01) Indicates presence of the service id member.
        HAS_SERVICE_ID = 0x01,

        // (0x02) Indicates presence of the qos member.
        HAS_QOS = 0x02,

        // (0x04) Indicates presence of the solicited flag.
        SOLICITED = 0x04,

        // (0x08) Indicates presence of the refresh complete flag.
        REFRESH_COMPLETE = 0x08,

        // (0x10) Indicates presence of the private stream flag.
        PRIVATE_STREAM = 0x10,

        // (0x20) Indicates presence of the clear cache flag.
        CLEAR_CACHE = 0x20

    }
}
