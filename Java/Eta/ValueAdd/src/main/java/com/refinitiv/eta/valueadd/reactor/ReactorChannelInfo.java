/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
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
     * Clears this object for reuse.
     */
    public void clear()
    {
        _chnlInfo.clear();
    }
}
