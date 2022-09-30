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
    public enum DictionaryRefreshFlags : int
    {
        // (0x00) No flags set.
        NONE = 0x00,

        // (0x01) When decoding, indicates presence of the dictionaryId, version,
        // and type members. This flag is not used while encoding as this
        // information is automatically added by the encode method when
        // appropriate.
        HAS_INFO = 0x01,

        // (0x02) When decoding, indicates that this is the final part of the
        // dictionary refresh. This flag is not used while encoding as it is
        // automatically set by the encode method set when appropriate.
        IS_COMPLETE = 0x02,

        // (0x04) Indicates that this refresh is being provided in response to a
        // request.
        SOLICITED = 0x04,

        // (0x08) Indicates presence of the sequenceNumber member.
        HAS_SEQ_NUM = 0x08,

        // (0x10) Indicates the receiver of the refresh should clear any existing
        // cached information.
        CLEAR_CACHE = 0x10
    }
}
