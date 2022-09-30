/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    [Flags]
    public enum DirectoryUpdateFlags : int
    {
        // (0x00) No flags set.
        NONE = 0x00,

        // (0x01) Indicates presence of the serviceId member.
        HAS_SERVICE_ID = 0x01,

        // (0x02) Indicates presence of the filter member.
        HAS_FILTER = 0x02,

        // (0x04) Indicates presence of the sequenceNumber member.
        HAS_SEQ_NUM = 0x04
    }
}
