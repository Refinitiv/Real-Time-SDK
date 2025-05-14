///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2025 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.List;

/**
 * ServiceEndpointDiscoveryInfo represents an service endpoint information from LDP service discovery.
 *
 * <p>This class is retrieved from ServiceEndpointDiscoveryResp</p>
 * 
 * @see ServiceEndpointDiscoveryResp
 */

public interface ServiceEndpointDiscoveryInfo
{
	/**
	 * Gets a list of data format supported by this endpoint.
	 * 
	 * @return a list of data format
	 */
	public List<String> dataFormatList();
	
	/**
	 * Gets an endpoint or domain name for establishing a connection
	 * 
	 * @return an endpoint
	 */
	public String endpoint();
	
	/**
	 * Gets a list of locations where the infrastructure is deployed in Real-Time - Optimized 
	 * 
	 * @return a list of location
	 */
	public List<String> locationList();
	
	/**
	 * Gets a port for establishing a connection
	 * 
	 * @return a port
	 */
	public String port();
	
	/**
	 * Gets a public Real-Time - Optimized provider
	 * 
	 * @return a Real-Time - Optimized provider
	 */
	public String provider();
	
	
	/**
	 * Gets a transport type
	 * 
	 * @return a transport type
	 */
	public String transport();
	
	/**
     * Gets string representation of this object
     * 
     * @return String representation of ServiceEndpointDiscoveryInfo.
     */
    public String toString();
}
