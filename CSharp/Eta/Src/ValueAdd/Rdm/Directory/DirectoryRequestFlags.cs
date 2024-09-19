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
    /// Directory Request Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="DirectoryRequest"/>
    [Flags]
    public enum DirectoryRequestFlags : int
    {
        /// <summary>
        /// (0x00) No flags set.
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// (0x01) Indicates whether the request is a streaming request, i.e. whether updates about source directory information are desired. 
        /// </summary>
        STREAMING = 0x01,

        /// <summary>
        /// (0x02) Indicates presence of the serviceId member.
        /// </summary>
        HAS_SERVICE_ID = 0x02
    }
}
