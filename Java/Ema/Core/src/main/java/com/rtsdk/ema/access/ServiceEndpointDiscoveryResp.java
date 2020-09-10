///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

import java.util.List;

/**
 * ServiceEndpointDiscoveryResp represents a response from EDP-RT service discovery which contains a list of ServiceEndpointDiscoveryInfo
 *
 * @see ServiceEndpointDiscoveryInfo
 */

public interface ServiceEndpointDiscoveryResp
{
	/**
	 * Gets a list of ServiceEndpointDiscoveryInfo of this ServiceEndpointDiscoveryResp.
	 * 
	 * @return a list of ServiceEndpointDiscoveryInfo
	 */
	List<ServiceEndpointDiscoveryInfo> serviceEndpointInfoList();
	
	/**
     * Gets string representation of this object
     * 
     * @return String representation of ServiceEndpointDiscoveryInfo.
     */
    public String toString();
}
