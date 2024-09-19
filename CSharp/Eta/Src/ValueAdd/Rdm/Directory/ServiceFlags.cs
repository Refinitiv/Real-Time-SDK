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
    /// Service Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="Service"/>
    [Flags]
    public enum ServiceFlags : int
    {
        /// <summary>
        /// (0x00) No flags set.
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// (0x01) Indicates presence of the info structure. See <see cref="ServiceInfo"/>.
        /// </summary>
        HAS_INFO = 0x01,

        /// <summary>
        /// (0x02) Indicates presence of the state structure. See <see cref="ServiceState"/>.
        /// </summary>
        HAS_STATE = 0x02,

        /// <summary>
        /// (0x04) Indicates presence of the service load structure. See <see cref="ServiceLoad"/>.
        /// </summary>
        HAS_LOAD = 0x04,

        /// <summary>
        /// (0x08) Indicates presence of the service data structure. See <see cref="ServiceData"/>.
        /// </summary>
        HAS_DATA = 0x08,

        /// <summary>
        /// (0x10) Indicates presence of the service link structure. See <see cref="ServiceLink"/>.
        /// </summary>
        HAS_LINK = 0x10
    }
}
