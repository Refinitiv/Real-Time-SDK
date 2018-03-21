package com.thomsonreuters.upa.perftools.upajtransportperf;

import com.thomsonreuters.upa.perftools.common.ClientChannelInfo;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;

/** Process message interface. Enables different ways to process a message. */
public interface ProcessMsg
{
	public int processMsg(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, TransportBuffer msgBuffer,	Error error);
}
