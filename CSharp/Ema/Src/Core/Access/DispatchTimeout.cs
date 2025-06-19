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
    /// This class defines additional dispatch timeout behaviors 
    /// </summary>
    public static class DispatchTimeout
    {
        /// <summary>
        /// Dispatch blocks till a message arrives.
        /// </summary>
        public const int INFINITE_WAIT = -1;

        /// <summary>
        /// Dispatch exits immediately even if there is no message.
        /// </summary>
        public const int NO_WAIT = 0;
    }
}
