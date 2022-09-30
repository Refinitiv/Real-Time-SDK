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
    public enum ServiceInfoFlags : int
    {
        // (0x000) No flags set.
        NONE = 0x0000,

        // (0x001) Indicates presence of the vendor member.
        HAS_VENDOR = 0x001,

        // (0x002) Indicates presence of the isSource member.
        HAS_IS_SOURCE = 0x002,

        // (0x004) Indicates presence of the dictionariesProvidedList member.
        HAS_DICTS_PROVIDED = 0x004,

        // (0x008) Indicates presence of the dictionariesUsedList member.
        HAS_DICTS_USED = 0x008,

        // (0x010) Indicates presence of the qosList member.
        HAS_QOS = 0x010,

        // (0x020) Indicates presence of the supportsQosRange member.
        HAS_SUPPORT_QOS_RANGE = 0x020,

        // (0x040) Indicates presence of the itemList member.
        HAS_ITEM_LIST = 0x040,

        // (0x080) Indicates presence of the supportsOutOfBandSnapshots member.
        HAS_SUPPORT_OOB_SNAPSHOTS = 0x080,

        // (0x100) Indicates presence of the acceptingConsumerStatus member.
        HAS_ACCEPTING_CONS_STATUS = 0x100
    }
}
