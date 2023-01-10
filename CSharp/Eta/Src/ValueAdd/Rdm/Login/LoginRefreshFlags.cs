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
    /// Login Refresh Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="LoginRefresh"/>
    [Flags]
    public enum LoginRefreshFlags
    {
        /// <summary>
        /// (0x00000) No flags set.
        /// </summary>
        NONE = 0x0000,

        /// <summary>
        /// (0x0001) Indicates whether the receiver of the refresh should clear any associated cache information.
        /// </summary>
        CLEAR_CACHE = 0x0001,

        /// <summary>
        /// (0x0002) Indicates presence of connectionconfig - payload for login
        /// </summary>
        /// refresh containing numStandbyServers and serverList members.
        HAS_CONN_CONFIG = 0x0002,

        /// <summary>
        /// (0x0004) Indicates presence of login attrib member.
        /// </summary>
        HAS_ATTRIB = 0x0004,

        /// <summary>
        /// (0x0008) Indicates presence of the sequenceNumber member.
        /// </summary>
        HAS_SEQ_NUM = 0x0008,

        /// <summary>
        /// (0x0010) Indicates presence of login support features member.
        /// </summary>
        HAS_FEATURES = 0x0010,

        /// <summary>
        /// (0x0020) Indicates presence of the userName member.
        /// </summary>
        HAS_USERNAME = 0x0020,

        /// <summary>
        /// (0x0040) Indicates presence of the userNameType member.
        /// </summary>
        HAS_USERNAME_TYPE = 0x0040,

        /// <summary>
        /// (0x0080) Indicates whether this refresh is being provided in response to a request.
        /// </summary>
        SOLICITED = 0x0080,

        //Reserve space for this flag
        //HAS_PROVIDER_SUPPORT_DICTIONARY_DOWNLOAD = 0x0100,

        /// <summary>
        /// (0x0200) Indicates presence of the authenticationTTReissue member.
        /// </summary>
        /// This is used when the userNameType member is set to
        /// ElementNames.AUTHN_TOKEN
        HAS_AUTHENTICATION_TT_REISSUE = 0x0200,

        /// <summary>
        /// (0x0400) Indicates presence of the authenticationExtendedResp member.
        /// </summary>
        /// This is optionally used when the userNameType member is set to
        /// ElementNames.AUTHN_TOKEN
        HAS_AUTHENTICATION_EXTENDED_RESP = 0x0400,

        /// <summary>
        /// (0x0800) Indicates presence of the authenticationErrorCode member.
        /// </summary>
        /// This is used when the userNameType member is set to
        /// ElementNames.AUTHN_TOKEN
        HAS_AUTHENTICATION_ERROR_CODE = 0x0800,

        /// <summary>
        /// (0x1000) Indicates presence of the authenticationErrorText member.
        /// </summary>
        /// This is used when the userNameType member is set to
        /// ElementNames.AUTHN_TOKEN
        HAS_AUTHENTICATION_ERROR_TEXT = 0x1000
    }
}
