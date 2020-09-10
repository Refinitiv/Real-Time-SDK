package com.rtsdk.eta.valueadd.reactor;

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
