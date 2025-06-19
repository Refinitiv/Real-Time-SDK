/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access;

/// <summary>
/// ServiceEndpointDiscoveryClient class provides callback interfaces to pass received responses.
/// <p>Application needs to implement an application client class inheriting from ServiceEndpointDiscoveryClient.
/// In its own class, application needs to override callback methods it desires to use for receiving responses.
/// Default empty callback methods are implemented by ServiceEndpointDiscoveryClient class.</p>
/// <p> Thread safety of all the methods in this class depends on the user's implementation.</p>
/// The following code snippet shows basic usage of ServiceEndpointDiscoveryClient class to print received responses to screen.
/// </summary>
public interface IServiceEndpointDiscoveryClient
{
    /// <summary>
    /// Invoked upon receiving a success response.
    /// </summary>
    /// <param name="serviceEndpointResp">serviceEndpointResp received ServiceEndpointDiscoveryResp.</param>
    /// <param name="serviceEndpointDiscoveryEvent">identifies open query for which this response is received.</param>
    void OnSuccess(ServiceEndpointDiscoveryResp serviceEndpointResp, ServiceEndpointDiscoveryEvent @serviceEndpointDiscoveryEvent);

    /// <summary>
    /// Invoked upon receiving an error response.
    /// </summary>
    /// <param name="errorText">received error text message.</param>
    /// <param name="event">identifies open query for which this response is received.</param>
    void OnError(string errorText, ServiceEndpointDiscoveryEvent @event);
}
