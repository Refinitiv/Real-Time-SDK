/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.nio.channels.SelectableChannel;
import java.util.ArrayList;
import java.util.List;

/**
 * Represents the Reactor warm standby channel information. See {@link ReactorChannel}.
 */
public class ReactorWarmStandbyChannelInfo 
{
	private List<SelectableChannel> selectableChannelList = new ArrayList<SelectableChannel>(); 	// A list of channels of the Reactor warm standby channel. Used for notification of available data for this channel.
	private List<SelectableChannel> oldSelectableChannelList = new ArrayList<SelectableChannel>(); // A list of previous channels of the Reactor warm standby channel. For if a change event has occurred.

	ReactorWarmStandbyChannelInfo()
	{
		
	}
	
	void clear()
	{
		selectableChannelList.clear();
		oldSelectableChannelList.clear();
	}
	
	
	/**
	 * Returns the list of SelectableChannels that are currently in use by this ReactorChannel.
	 * @return List of selectableChannels
	 */
	public List<SelectableChannel> selectableChannelList()
	{
		return selectableChannelList;
	}
	
	
	/**
	 * Returns the list of old SelectableChannels that are that were in use by this channel before now. 
	 * Channels that are in this list but not in the selectableChannelList can be removed from any selector or notifier.
	 * @return List of old selectableChannels
	 */
	public List<SelectableChannel> oldSelectableChannelList()
	{
		return oldSelectableChannelList;
	}
}
