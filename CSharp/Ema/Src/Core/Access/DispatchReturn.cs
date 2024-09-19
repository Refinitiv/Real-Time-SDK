/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// This class defines the Dispatch method returns values.
    /// </summary>
    public static class DispatchReturn
    {
        /// <summary>
        /// Dispatch exits immediately even if there is no message.
        /// </summary>
        public const int TIMEOUT = 0;

        /// <summary>
        /// A message was dispatched on this dispatch call.
        /// </summary>
        public const int DISPATCHED = 1;
    }
}
