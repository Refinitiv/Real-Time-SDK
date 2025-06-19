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
    /// Dictionary Refresh Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="DictionaryRefresh"/>
    [Flags]
    public enum DictionaryRefreshFlags : int
    {
        /// <summary>
        /// (0x00) No flags set.
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// (0x01) When decoding, indicates presence of the dictionaryId, version, and type members. This flag is not used while encoding as this
        /// information is automatically added by the encode method when appropriate.
        /// </summary>
        HAS_INFO = 0x01,

        /// <summary>
        /// (0x02) When decoding, indicates that this is the final part of the dictionary refresh. This flag is not used while encoding as it is
        /// automatically set by the encode method set when appropriate.
        /// </summary>
        IS_COMPLETE = 0x02,

        /// <summary>
        /// (0x04) Indicates that this refresh is being provided in response to a request.
        /// </summary>
        SOLICITED = 0x04,

        /// <summary>
        /// (0x08) Indicates presence of the sequenceNumber member.
        /// </summary>
        HAS_SEQ_NUM = 0x08,

        /// <summary>
        /// (0x10) Indicates the receiver of the refresh should clear any existing cached information.
        /// </summary>
        CLEAR_CACHE = 0x10
    }
}
