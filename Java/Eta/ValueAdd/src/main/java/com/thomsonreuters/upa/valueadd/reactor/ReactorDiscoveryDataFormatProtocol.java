package com.thomsonreuters.upa.valueadd.reactor;

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
	public static final int RSSL_RD_DP_INIT = 0;
	/** Rwf data format protocol */    	
	public static final int RSSL_RD_DP_RWF = 1;
	/** tr_json2 data format protocol */    	
	public static final int RSSL_RD_DP_JSON2 = 2;
}
