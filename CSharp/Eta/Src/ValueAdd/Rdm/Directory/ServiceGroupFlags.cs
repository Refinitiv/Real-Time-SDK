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
    /// Service Group Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// /// <seealso cref="ServiceGroup"/>
    [Flags]
    public enum ServiceGroupFlags : int
    {
        /// <summary>
        /// (0x00) No flags set.
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// (0x01) Indicates that this structure has a Merged To Group buffer.
        /// </summary>
        HAS_MERGED_TO_GROUP = 0x01,

        /// <summary>
        /// (0x02) Indicates the presence of a Status field.
        /// </summary>
        HAS_STATUS = 0x02
    }
}
