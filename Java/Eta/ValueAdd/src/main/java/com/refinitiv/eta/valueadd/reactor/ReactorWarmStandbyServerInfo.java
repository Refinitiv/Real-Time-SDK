/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Configuration options for creating a Warm Standby server information. See {@link ReactorWarmStandbyGroup}.
 */
public class ReactorWarmStandbyServerInfo 
{
	
	private ReactorConnectInfo reactorConnectInfo;	// Reactor Connect Info
	
	/*
	 * Per service based option for selecting a list of active services for this server.
	 * The first connected server provides active services if this option is not configured.
	 */
	private ReactorPerServiceBasedOptions perServiceBasedOptions;
	
	private boolean isActiveChannelConfig;
	
	
	
	ReactorWarmStandbyServerInfo()
	{
		reactorConnectInfo = ReactorFactory.createReactorConnectInfo();
		perServiceBasedOptions = new ReactorPerServiceBasedOptions();
		isActiveChannelConfig = true;
	}
	
	/**
	 * Clears this object.
	 */
	public void clear()
	{
		reactorConnectInfo.clear();
		perServiceBasedOptions.clear();
		isActiveChannelConfig = true;
	}
	
	/**
	 * Reactor connection information for use by this warm standby server.
	 * @return ReactorConnectInfo object
	 */
	public ReactorConnectInfo reactorConnectInfo()
	{
		return reactorConnectInfo;
	}
	
	/**
	 * Deep copies the provided reactorConnectInfo
	 * @param reactorConnectInfo ReactorConnectInfo object
	 */
	public void reactorConnectInfo(ReactorConnectInfo reactorConnectInfo)
	{
		reactorConnectInfo.copy(this.reactorConnectInfo);
	}
	
	/**
	 * Returns the perServiceBasedOptions object, which contains per service configuration for this connection.
	 * @return ReactorPerServiceBasedOptions object.
	 */
	public ReactorPerServiceBasedOptions perServiceBasedOptions()
	{
		return perServiceBasedOptions;
	}
	
	/**
	 * Deep copies the perServiceBasedOptions object, which contains per service configuration for this connection.
	 * @param perServiceBasedOptions ReactorPerServiceBasedOptions object
	 */
	public void perServiceBasedOptions(ReactorPerServiceBasedOptions perServiceBasedOptions)
	{
		perServiceBasedOptions.copy(this.perServiceBasedOptions);
	}

	/**
	 * Deep copies this object to destServer
	 * @param destServer ReactorWarmStandbyServerInfo object
	 */
	public void copy(ReactorWarmStandbyServerInfo destServer) {
		destServer.clear();
		perServiceBasedOptions.copy(destServer.perServiceBasedOptions);
		reactorConnectInfo.copy(destServer.reactorConnectInfo);
	}
	
	
	/*
	 * Internal function used by the reactor
	 */
	void isActiveChannelConfig(boolean isActiveChannelConfig)
	{
		this.isActiveChannelConfig = isActiveChannelConfig;
	}
	
	/*
	 * Internal function used by the reactor
	 */
	boolean isActiveChannelConfig()
	{
		return isActiveChannelConfig;
	}
	
}
