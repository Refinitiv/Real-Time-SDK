package com.refinitiv.eta.transport;

/**
 * Options used for configuring WebSocket specific transport options.
 * 
 * @see ConnectOptions
 */

public interface WSocketOpts
{
	/**
	 * Specifies a list of protocols to be negotiated between WebSocket server and client.
	 * <p>The left-to-right priority ordered, white space delineated list of supported/preferred protocols</p> 
	 * 
	 * @param protocolList the list of preferred protocols
	 */
	void protocols(String protocolList);
	
	/**
	 * The protocol list to be negotiated between WebSocket server and client. 
	 * 
	 * @return the protocol list 
	 */
	String protocols();
	
	/**
	 * Specifies a maximum message size for reading/sending messages on WebSocket client.
	 * 
	 * @param maxMsgSize the maximum message size
	 */
	void maxMsgSize(long maxMsgSize);
	
	/*
	 * The maximum message size for reading/sending messages on WebSocket client.
	 * 
	 * @return the maximum message size
	 */
	long maxMsgSize();

	/**
	 * Apply callback function for accessing to WS Handshake request and response.
	 * @param httpCallback the callback function
	 */
	void httpCallback(HttpCallback httpCallback);

	/**
	 * The callback function for accessing to WS handshake request and response.
	 * @return the callback function
	 */
	HttpCallback httpCallback();

	void clear();
}
