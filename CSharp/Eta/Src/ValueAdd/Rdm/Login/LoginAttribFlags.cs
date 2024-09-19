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
    /// Login Attrib Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="LoginAttrib"/>
    [Flags]
    public enum LoginAttribFlags : int
    {
        /// <summary>
        /// (0x0000) No flags set
        /// </summary>
        NONE = 0x0000,

        /// <summary>
        /// (0x0001) Indicates presence of the allowSuspectData member.
        /// </summary>
        HAS_ALLOW_SUSPECT_DATA = 0x0001,

        /// <summary>
        /// (0x0002) Indicates presence of the applicationId member
        /// </summary>
        HAS_APPLICATION_ID = 0x0002,

        /// <summary>
        /// (0x0004) Indicates presence of the applicationName member
        /// </summary>
        HAS_APPLICATION_NAME = 0x0004,

        /// <summary>
        /// (0x0008) Indicates presence of the position member
        /// </summary>
        HAS_POSITION = 0x0008,

        /// <summary>
        /// (0x0020) Indicates presence of the providePermissionExpressions member
        /// </summary>
        HAS_PROVIDE_PERM_EXPR = 0x0020,

        /// <summary>
        /// (0x0040) Indicates presence of the providePermissionProfile member
        /// </summary>
        HAS_PROVIDE_PERM_PROFILE = 0x0040,

        /// <summary>
        /// (0x0080) Indicates presence of the singleOpen member
        /// </summary>
        HAS_SINGLE_OPEN = 0x0080,

        /// <summary>
        /// (0x0100) Inform a Provider that it can request dictionary.
        /// </summary>
        /// Support for this request is indicated by the
        /// supportProviderDictionaryDownload member of the LoginAttrib
        HAS_PROVIDER_SUPPORT_DICTIONARY_DOWNLOAD = 0x0100,

        /// <summary>
        /// (0x4000) Inform a Provider that consumer support handling of RTT messages.
        /// </summary>
        HAS_CONSUMER_SUPPORT_RTT = 0x4000
    }
}
