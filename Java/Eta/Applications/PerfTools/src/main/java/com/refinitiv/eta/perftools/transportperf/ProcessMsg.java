package com.refinitiv.eta.perftools.transportperf;

import com.refinitiv.eta.perftools.common.ClientChannelInfo;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;

/** Process message interface. Enables different ways to process a message. */
public interface ProcessMsg
{
	public int processMsg(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, TransportBuffer msgBuffer,	Error error);
}
