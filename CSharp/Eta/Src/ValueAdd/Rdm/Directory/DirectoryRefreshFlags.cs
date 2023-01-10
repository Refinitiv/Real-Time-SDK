/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Directory Refresh Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="DirectoryRefresh"/>
    [Flags]
    public enum DirectoryRefreshFlags : int
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
        /// (0x02) Indicates whether this Refresh is provided in response to a request.
        /// </summary>
        SOLICITED = 0x02,

        /// <summary>
        /// (0x04) Indicates presence of the sequenceNumber member.
        /// </summary>
        HAS_SEQ_NUM = 0x04,

        /// <summary>
        /// (0x08) Indicates whether the Consumer should clear any existing cached service information.
        /// </summary>
        CLEAR_CACHE = 0x08
    }
}
