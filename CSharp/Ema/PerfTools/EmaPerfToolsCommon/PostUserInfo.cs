/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.PerfTools.Common
{
    /// <summary>
    /// Helper class for storing Post Iser Info
    /// </summary>
    public class PostUserInfo
    {
        /// <summary>
        /// User Address
        /// </summary>
        public long UserAddr { get; set; }

        /// <summary>
        /// User Id
        /// </summary>
        public long UserId { get; set; }
    }
}
