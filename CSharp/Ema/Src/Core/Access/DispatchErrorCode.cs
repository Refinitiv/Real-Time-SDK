/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Dispatch error codes from Reactor.Dispatch method.
    /// </summary>
    public class DispatchErrorCode
    {
        /// <summary>
        /// General failure.
        /// </summary>
        public const int FAILURE = -1;

        /// <summary>
        /// Reactor is shutdown.
        /// </summary>
        public const int SHUTDOWN = -10;
    }
}
