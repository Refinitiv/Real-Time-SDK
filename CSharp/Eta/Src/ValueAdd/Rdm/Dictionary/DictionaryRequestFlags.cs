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
    /// Dictionary Request Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="DictionaryRequest"/>
    [Flags]
    public enum DictionaryRequestFlags : int
    {
        /// <summary>
        /// (0x0) No flags set.
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// (0x1) Indicates this is to be a streaming request.
        /// </summary>
        STREAMING = 0x01
    }
}
