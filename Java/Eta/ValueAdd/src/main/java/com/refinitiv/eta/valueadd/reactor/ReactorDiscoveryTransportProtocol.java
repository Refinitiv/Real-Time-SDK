/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Types indicating the transport query parameter.
 * @see ReactorServiceDiscoveryOptions
 */
public class ReactorDiscoveryTransportProtocol 
{
    /**
     * Instantiates a new transport protocol.
     */
    private ReactorDiscoveryTransportProtocol()
    {
        throw new AssertionError();
    }
    
	/** Unknown transport protocol */
	public static final int RD_TP_INIT = 0;
	/** TCP transport protocol */    	
	public static final int RD_TP_TCP = 1;
	/** Websocket transport protocol */  
	public static final int RD_TP_WEBSOCKET = 2;  
	
	
}
