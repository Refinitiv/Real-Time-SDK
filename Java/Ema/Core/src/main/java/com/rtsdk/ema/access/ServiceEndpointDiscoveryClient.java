///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

/**
 * ServiceEndpointDiscoveryClient class provides callback interfaces to pass received responses.
 * 
 * <p>Application needs to implement an application client class inheriting from ServiceEndpointDiscoveryClient.<br>
 * In its own class, application needs to override callback methods it desires to use for receiving responses.<br>
 * Default empty callback methods are implemented by ServiceEndpointDiscoveryClient class.</p>
 * 
 * <p>Thread safety of all the methods in this class depends on the user's implementation.</p>
 * 
 * The following code snippet shows basic usage of ServiceEndpointDiscoveryClient class to print received responses to screen.
 * 
 * <pre>
 * class AppClient implements ServiceEndpointDiscoveryClient
 * {
 * 		public void onSuccess(ServiceEndpointDiscoveryResp serviceEndpointResp, ServiceEndpointDiscoveryEvent event)
 * 		{
 * 			System.out.println(serviceEndpointResp);
 * 		}
 * 
 * 		public void onError(String errorText, ServiceEndpointDiscoveryEvent event)
 * 		{
 * 			System.out.println(errorText);
 * 		}
 * }
 * </pre>
 * 
 * @see ServiceEndpointDiscovery
 * @see ServiceEndpointDiscoveryResp
 * @see ServiceEndpointDiscoveryEvent
 */
public interface ServiceEndpointDiscoveryClient
{
	/**
	 * Invoked upon receiving a success response.
	 * 
	 * @param serviceEndpointResp received ServiceEndpointDiscoveryResp
	 * @param event identifies open query for which this response is received
	 */
	void onSuccess(ServiceEndpointDiscoveryResp serviceEndpointResp, ServiceEndpointDiscoveryEvent event);
	
	/**
	 * Invoked upon receiving an error response.
	 * 
	 * @param errorText received error text message
	 * @param event identifies open query for which this response is received
	 */
	void onError(String errorText, ServiceEndpointDiscoveryEvent event);
}
