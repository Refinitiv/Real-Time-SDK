package com.refinitiv.eta.valueadd.reactor;

public class ReactorWSocketAcceptOptions
{
	boolean sendPingMessage;
	
	ReactorWSocketAcceptOptions()
	{
		clear();
	}
	
	void clear()
	{
		sendPingMessage = true;
	}

	/**
	 * This is used to configure the Reactor to periodically send a ping message to clients.
	 * <p>Defaults to true</p>
	 * 
	 * @param sendPing specifies true to send a JSON ping message.
	 */
	public void sendPingMessage(boolean sendPing)
	{
		sendPingMessage = sendPing;
	}
}
