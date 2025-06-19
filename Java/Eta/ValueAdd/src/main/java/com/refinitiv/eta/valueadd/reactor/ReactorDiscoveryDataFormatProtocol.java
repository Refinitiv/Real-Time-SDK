/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Types indicating the dataformat query parameter.
 * @see ReactorServiceDiscoveryOptions
 */
public class ReactorDiscoveryDataFormatProtocol 
{
    /**
     * Instantiates a new dataformat protocol.
     */
    private ReactorDiscoveryDataFormatProtocol()
    {
        throw new AssertionError();
    }	
	
	/** Unknown data format */    	
	public static final int RD_DP_INIT = 0;
	/** Rwf data format protocol */    	
	public static final int RD_DP_RWF = 1;
	/** tr_json2 data format protocol */    	
	public static final int RD_DP_JSON2 = 2;
}
