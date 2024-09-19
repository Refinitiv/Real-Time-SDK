/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Directory Update Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="DirectoryUpdate"/>
    [Flags]
    public enum DirectoryUpdateFlags : int
    {
        /// <summary>
        /// (0x00) No flags set.
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// (0x01) Indicates presence of the serviceId member.
        /// </summary>
        HAS_SERVICE_ID = 0x01,

        /// <summary>
        /// (0x02) Indicates presence of the filter member.
        /// </summary>
        HAS_FILTER = 0x02,

        /// <summary>
        /// (0x04) Indicates presence of the sequenceNumber member.
        /// </summary>
        HAS_SEQ_NUM = 0x04
    }
}
