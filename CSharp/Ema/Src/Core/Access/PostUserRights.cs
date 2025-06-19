/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// PostUserRights represents post user rights.
    /// </summary>
    public enum PostUserRights
    {
        /// <summary>
        /// Specifies ability to create records.
        /// </summary>
        CREATE = 0x01,
        /// <summary>
        /// Specifies ability to delete records.
        /// </summary>
        DELETE = 0x02,
        /// <summary>
        /// Specifies ability to modify permissions.
        /// </summary>
        MODIFY_PERMISSION = 0x04
    }
}
