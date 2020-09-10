package com.rtsdk.eta.shared;

import com.rtsdk.eta.transport.Channel;
import com.rtsdk.eta.transport.TransportBuffer;

/**
 * Callback method used by {@link ProviderSession#read(Channel, com.rtsdk.eta.transport.Error, ReceivedMsgCallback)} to process client request.
 */
public interface ReceivedMsgCallback
{
    public void processReceivedMsg(Channel channel, TransportBuffer msgBuf);

    public void processChannelClose(Channel channel);
}
