package com.rtsdk.eta.perftools.upajtransportperf;

import com.rtsdk.eta.perftools.common.ClientChannelInfo;
import com.rtsdk.eta.transport.Error;
import com.rtsdk.eta.transport.TransportBuffer;

/** Process message interface. Enables different ways to process a message. */
public interface ProcessMsg
{
	public int processMsg(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, TransportBuffer msgBuffer,	Error error);
}
