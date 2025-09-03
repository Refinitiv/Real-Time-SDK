/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.valueadd.reactor.ReactorChannel;

/**
 * Base class of {@link ConsumerSessionInfo} and {@link ProviderSessionInfo}.
 *
 * @see ConsumerSessionInfo
 * @see ProviderSessionInfo
 */
public abstract class SessionInfo {

    protected ChannelInformationImpl channelInformation = new ChannelInformationImpl();

    /**
     * Returns the {@link ChannelInformation} for this session.
     * @return channel information associated with this session.
     */
    public ChannelInformation getChannelInformation() {
        return channelInformation;
    }

    protected void loadSessionInfo(ReactorChannel channel) {
        channelInformation.clear();
        
        if(channel == null)
        	return;
        
        /* Checks whether the SessionChannelInfo is available */
        if(channel.userSpecObj() != null && channel.userSpecObj() instanceof ChannelInfo)
        {
        	ChannelInfo chnlInfo = (ChannelInfo)channel.userSpecObj();
        	
        	if(chnlInfo.sessionChannelInfo() != null)
        	{
        		channelInformation.set(channel, chnlInfo.sessionChannelInfo().sessionChannelConfig());
        	}
        	else
        	{
        		channelInformation.set(channel, null);
        	}
        	
        	channelInformation.channelName(chnlInfo._channelConfig.name);
        }
        else
        {
        	channelInformation.set(channel, null);
        }
    }
}
