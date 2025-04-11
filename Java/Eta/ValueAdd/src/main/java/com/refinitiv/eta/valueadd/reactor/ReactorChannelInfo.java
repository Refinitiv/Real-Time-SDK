/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022,2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.transport.ChannelInfo;
import com.refinitiv.eta.transport.TransportFactory;

/**
 * Information returned by the {@link ReactorChannel#info(ReactorChannelInfo, ReactorErrorInfo)} call.
 */
public class ReactorChannelInfo
{
    private ChannelInfo _chnlInfo = TransportFactory.createChannelInfo();
    private ReactorPreferredHostInfo _preferredHostInfo = new ReactorPreferredHostInfo();
 
    /**
     *  Channel information.
     *
     * @return the channel info
     */
    public ChannelInfo channelInfo()
    {
        return _chnlInfo;
    }
     
    /**
     * Preferred host information.
     * @return the preferred host information
     */
    public ReactorPreferredHostInfo preferredHostInfo()
    {
        return _preferredHostInfo;
    }
    
    /**
     * Sets the preferred host information based on configured options.
     * @param the preferred host options
     */
    public void preferredHostInfo(ReactorPreferredHostOptions preferredHostOptions)
    {
    	_preferredHostInfo._connectionListIndex = preferredHostOptions.connectionListIndex();
    	_preferredHostInfo._detectionTimeInterval = preferredHostOptions.detectionTimeInterval();
    	_preferredHostInfo._detectionTimeSchedule = preferredHostOptions.detectionTimeSchedule();
    	_preferredHostInfo._fallBackWithInWSBGroup = preferredHostOptions.fallBackWithInWSBGroup();
    	_preferredHostInfo._isPreferredHostEnabled = preferredHostOptions.isPreferredHostEnabled();
    	_preferredHostInfo._warmStandbyGroupListIndex = preferredHostOptions.warmStandbyGroupListIndex();
    }

    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
    	_chnlInfo.clear();
        _preferredHostInfo.clear();
    }
}
