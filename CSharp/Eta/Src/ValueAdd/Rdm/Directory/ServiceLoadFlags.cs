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
    /// Service Info Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="ServiceLoad"/>
    [Flags]
    public enum ServiceLoadFlags : int
    {
        /// <summary>
        /// (0x00) No flags set.
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// (0x01) Indicates presence of an open limit on the service.
        /// </summary>
        HAS_OPEN_LIMIT = 0x01,

        /// <summary>
        /// (0x02) Indicates presence of an open window on the service.
        /// </summary>
        HAS_OPEN_WINDOW = 0x02,

        /// <summary>
        /// (0x04) Indicates presence of a load factor.
        /// </summary>
        HAS_LOAD_FACTOR = 0x04
    }
}
