/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Login Status Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="ServerInfo"/>
    [Flags]
    public enum ServerInfoFlags
    {
        /// <summary>
        /// (0x00) No flags set
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// (0x01) Indicates presence of the loadFactor member.
        /// </summary>
        HAS_LOAD_FACTOR = 0x01,

        /// <summary>
        /// (0x02) Indicates presence of the serverType member.
        /// </summary>
        HAS_TYPE = 0x02
    }
}
