/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Service Info Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="ServiceInfo"/>
    [Flags]
    public enum ServiceInfoFlags : int
    {
        /// <summary>
        /// (0x000) No flags set.
        /// </summary>
        NONE = 0x0000,

        /// <summary>
        /// (0x001) Indicates presence of the vendor member.
        /// </summary>
        HAS_VENDOR = 0x001,

        /// <summary>
        /// (0x002) Indicates presence of the isSource member.
        /// </summary>
        HAS_IS_SOURCE = 0x002,

        /// <summary>
        /// (0x004) Indicates presence of the dictionariesProvidedList member.
        /// </summary>
        HAS_DICTS_PROVIDED = 0x004,

        /// <summary>
        /// (0x008) Indicates presence of the dictionariesUsedList member.
        /// </summary>
        HAS_DICTS_USED = 0x008,

        /// <summary>
        /// (0x010) Indicates presence of the qosList member.
        /// </summary>
        HAS_QOS = 0x010,

        /// <summary>
        /// (0x020) Indicates presence of the supportsQosRange member.
        /// </summary>
        HAS_SUPPORT_QOS_RANGE = 0x020,

        /// <summary>
        /// (0x040) Indicates presence of the itemList member.
        /// </summary>
        HAS_ITEM_LIST = 0x040,

        /// <summary>
        /// (0x080) Indicates presence of the supportsOutOfBandSnapshots member.
        /// </summary>
        HAS_SUPPORT_OOB_SNAPSHOTS = 0x080,

        /// <summary>
        /// (0x100) Indicates presence of the acceptingConsumerStatus member.
        /// </summary>
        HAS_ACCEPTING_CONS_STATUS = 0x100
    }
}
