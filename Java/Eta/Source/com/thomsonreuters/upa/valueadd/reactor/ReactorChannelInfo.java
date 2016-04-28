package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.TransportFactory;

/**
 * Information returned by the {@link ReactorChannel#info(ReactorChannelInfo, ReactorErrorInfo)} call.
 */
public class ReactorChannelInfo
{
	private ChannelInfo _chnlInfo = TransportFactory.createChannelInfo();

	/** Channel information. */
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
