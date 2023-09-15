/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Common
{
    /// <summary>
    /// Flags for notifier events
    /// </summary>
    public enum NotifierEventFlag
    {
        /// <summary>
        /// Indicates Socket can be read from
        /// </summary>
        READ = 0x1,

        /// <summary>
        /// Indicates Socket can be written to
        /// </summary>
        WRITE = 0x2,

        /// <summary>
        /// Indicates Socket may be invalid.
        /// </summary>
        BAD_SOCKET = 0x4
    }
}
