/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>The RDM login request flags.</summary>
    ///
    /// <seealso cref="LoginRequest"/>
    [Flags]
    public enum LoginRequestFlags : int
    {
        /// <summary>
        /// (0x0000) No flags set
        /// </summary>
        NONE = 0x0000,

        /// <summary>
        /// (0x0001) Indicates presence of the attrib member.
        /// </summary>
        HAS_ATTRIB = 0x0001,

        /// <summary>
        /// (0x0002) Indicates presence of the downloadConnectionConfig member
        /// </summary>
        HAS_DOWNLOAD_CONN_CONFIG = 0x0002,

        /// <summary>
        /// (0x0004) Indicates presence of the instanceId member
        /// </summary>
        HAS_INSTANCE_ID = 0x0004,

        /// <summary>
        /// (0x0008) Indicates presence of the password member
        /// </summary>
        HAS_PASSWORD = 0x0008,

        /// <summary>
        /// (0x0010) Indicates presence of the role member
        /// </summary>
        HAS_ROLE = 0x0010,

        /// <summary>
        /// (0x0020) Indicates presence of the userNameType member
        /// </summary>
        HAS_USERNAME_TYPE = 0x0020,

        /// <summary>
        /// (0x0040) Indicates the Consumer or Non-Interactive provider does not
        /// </summary>
        /// require a refresh.
        NO_REFRESH = 0x0040,

        /// <summary>
        /// (0x0080) Used by a Consumer to request that all open items on a channel be paused.
        /// </summary>
        /// Support for this request is indicated by the
        /// supportOptimizedPauseResume member of the LoginRefresh
        PAUSE_ALL = 0x0080,

        /// <summary>
        /// (0x0200) Indicates presence of the authentication extended data member
        /// </summary>
        /// This is optionally used when the userNameType member is set to
        /// ElementNames.AUTHN_TOKEN
        HAS_AUTHENTICATION_EXTENDED = 0x0200
    }
}
