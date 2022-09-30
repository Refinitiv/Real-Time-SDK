/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// The authentication token callback is used to communicate token information to the application.
    /// </summary>
    public interface IReactorAuthTokenEventCallback
    {
        /// <summary>
        /// A callback function for applications to receive authentication token information from the token service.
        /// </summary>
        /// <param name="reactorAuthTokenEvent">A ReactorAuthTokenEvent containing event information.</param>
        /// <returns>A callback return code that can trigger specific Reactor behavior based on the outcome of the 
        /// callback function</returns>
        public ReactorCallbackReturnCode ReactorAuthTokenEventCallback(ReactorAuthTokenEvent reactorAuthTokenEvent); 
    }
}
