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
    /// Service State Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="ServiceState"/>
    [Flags]
    public enum ServiceStateFlags : int
    {
        /// <summary>
        /// (0x00) No flags set.
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// (0x01) Indicates presence of the acceptingRequests member.
        /// </summary>
        HAS_ACCEPTING_REQS = 0x01,

        /// <summary>
        /// (0x02) Indicates presence of the status member.
        /// </summary>
        HAS_STATUS = 0x02
    }
}
