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
