/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// The Reactor service discovery event callback is used to communicate service endpoint information to the application.
    /// </summary>
    public interface IReactorServiceEndpointEventCallback
    {
        /// <summary>
        /// A callback function for applications to receive endpoint information from the service discovery service.
        /// </summary>
        /// <param name="serviceEndpointEvent">A serviceEndpointEvent containing event information.</param>
        /// <returns>A callback return code that can trigger specific Reactor behavior based on the outcome of the 
        /// callback function</returns>
        public ReactorCallbackReturnCode ReactorServiceEndpointEventCallback(ReactorServiceEndpointEvent serviceEndpointEvent);
    }
}
