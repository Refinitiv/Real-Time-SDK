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
    /// Login Status Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="LoginStatus"/>
    [Flags]
    public enum LoginStatusFlags : int
    {
        /// <summary>
        /// (0x00) No flags set.
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// (0x01) Indicates presence of the state member.
        /// </summary>
        HAS_STATE = 0x01,

        /// <summary>
        /// (0x02) Indicates presence of the userName member.
        /// </summary>
        HAS_USERNAME = 0x02,

        /// <summary>
        /// (0x04) Indicates presence of the userNameType member.
        /// </summary>
        HAS_USERNAME_TYPE = 0x04,

        /// <summary>
        /// (0x08) Indicates whether the receiver of the login status should clear any associated cache information.
        /// </summary>
        CLEAR_CACHE = 0x08,

        /// <summary>
        /// (0x0010) Indicates presence of the authenticationErrorCode member.
        /// </summary>
        /// This is used when the userNameType member is set to
        /// ElementNames.AUTHN_TOKEN
        HAS_AUTHENTICATION_ERROR_CODE = 0x0010,

        /// <summary>
        /// (0x0020) Indicates presence of the authenticationErrorText member.
        /// </summary>
        /// This is used when the userNameType member is set to
        /// ElementNames.AUTHN_TOKEN
        HAS_AUTHENTICATION_ERROR_TEXT = 0x0020
    }
}
