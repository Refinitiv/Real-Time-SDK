/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

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
