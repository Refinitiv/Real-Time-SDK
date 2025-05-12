///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * ServiceEndpointDiscovery class provides the functionality to query endpoints from LDP service discovery.
 *
 * <p>Application interacts with the service discovery through the ServiceEndpointDiscovery interface methods.<br>
 * The results of these interactions are communicated back to application through ServiceEndpointDiscoveryClient.</p>
 * 
 * <p>An ServiceEndpointDiscovery is created from EmaFactory<br>
 * (see {@link com.refinitiv.ema.access.EmaFactory#createServiceEndpointDiscovery()}
 *  or {@link com.refinitiv.ema.access.EmaFactory#createServiceEndpointDiscovery(String, String)}).</p>
 *
 * The following code snippet shows basic usage of ServiceEndpointDiscovery class in a simple consumer type app.
 *
 * <pre>
 * // create an implementation for ServiceEndpointDiscoveryClient to process service discovery responses
 * class AppClient implements ServiceEndpointDiscoveryClient
 * {
 * 		void onSuccess(ServiceEndpointDiscoveryResp serviceEndpointDiscoveryResp, ServiceEndpointDiscoveryEvent event);
 *		void onError(String errorText, ServiceEndpointDiscoveryEvent event);
 * }
 * 
 * public class Consumer
 * {
 *		public static void main(String[] args)
 *		{
 *			ServiceEndpointDiscovery serviceEndpointDiscovery = null;
 *			try
 *			{
 *				AppClient appClient = new AppClient();
 *
 *				// instantiate ServiceEndpointDiscovery object
 *				serviceEndpointDiscovery = EmaFactory.createServiceEndpointDiscovery();
 *
 *				// Query endpoints from LDP service discovery
 *				serviceEndpointDiscovery.registerClient(EmaFactory.createServiceEndpointDiscoveryOpion().username(userName)
 *							.password(password).transport(ServiceEndpointDiscoveryOption::TCP).clientId(clientId), appClient);
 *			}
 *			catch (OmmException excp)
 *       	{
 *          	System.out.println(excp.getMessage());
 *       	}
 *       	finally 
 *       	{
 *          	if (serviceEndpointDiscovery != null) serviceEndpointDiscovery.uninitialize();
 *       	}
 *		}
 * }
 * </pre>
 * 
 * @see ServiceEndpointDiscoveryOption
 * @see ServiceEndpointDiscoveryClient
 * @see OmmException
 */
public interface ServiceEndpointDiscovery
{
	/** 
	 * Queries the LDP service discovery synchronously to get endpoints according to the specified parameters
	 * 
	 * <p>This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidUsageException if application passes invalid ServiceEndpointDiscoveryOption
	 * 
	 * @param params specifies query options to get endpoints
	 * @param client specifies ServiceEndpointDiscoveryClient instance receiving notifications about this query
	 */
	public void registerClient(ServiceEndpointDiscoveryOption params, ServiceEndpointDiscoveryClient client);
	
	/** 
	 * Queries the LDP service discovery synchronously to get endpoints according to the specified parameters
	 * 
	 * <p>This method is ObjectLevelSafe.</p>
	 * 
	 * @throws OmmInvalidUsageException if application passes invalid ServiceEndpointDiscoveryOption
	 * 
	 * @param params specifies query options to get endpoints
	 * @param client specifies ServiceEndpointDiscoveryClient instance receiving notifications about this query
	 * @param closure specifies application defined query identification
	 */
	public void registerClient(ServiceEndpointDiscoveryOption params, ServiceEndpointDiscoveryClient client, Object closure);
	
	/**
	 * Uninitializes the ServiceEndpointDiscovery object.
	 * 
	 * <p> This method is ObjectLevelSafe.</p>
	 */
	public void uninitialize();
}
