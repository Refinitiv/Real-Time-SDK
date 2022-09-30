/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.List;

/**
 * Reactor Warm Standby group configuration class.  This is used in {@link ReactorConnectOptions} to define Warm Standby connections.
 *
 */

public interface ReactorWarmStandbyGroup {
	
	/**
	 * Returns the startingActiveServer information, which defines the connection options for the initial starting server connection.
	 * The {@link ReactorWarmStandbyServerInfo} contains ReactorConnectInfo and an optional list of service name strings for this connection's
	 * configured services when used with Service-based Warm Standby. 
	 * @return the initial active connection's ReactorWarmStandbyServerInfo
	 */
	ReactorWarmStandbyServerInfo startingActiveServer();
	
	/**
	 * List of standby server connections.  Any standby server {@link ReactorWarmStandbyServerInfo} configuration need to placed in this list.
	 * Upon connection, the reactor will connect to each of these servers in the order that they were added to the list.
	 * @return Standby server list reference.
	 */
	List<ReactorWarmStandbyServerInfo> standbyServerList();
	
	/**
	 * Current warm standby mode for this warm standby group.  See {@link ReactorWarmStandbyMode}.
	 * @return integer value of the warm standby mode.
	 */
	int warmStandbyMode();
	
	/**
	 * Sets the warm standby mode for this warm standby group.  See {@link ReactorWarmStandbyMode} for valid inputs.
	 *
	 * @param warmStandbyMode the Warm Standby Mode
	 */
	void warmStandbyMode(int warmStandbyMode);
	
	
	/**
	 * Clears this ReactorWarmStandbyGroup.
	 */
	void clear();
		
}
