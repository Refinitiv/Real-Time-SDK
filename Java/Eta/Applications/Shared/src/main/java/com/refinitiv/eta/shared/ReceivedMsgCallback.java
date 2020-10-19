package com.refinitiv.eta.shared;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.TransportBuffer;

/**
 * Callback method used by {@link ProviderSession#read(Channel, com.refinitiv.eta.transport.Error, ReceivedMsgCallback)} to process client request.
 */
public interface ReceivedMsgCallback
{
    public void processReceivedMsg(Channel channel, TransportBuffer msgBuf);

    public void processChannelClose(Channel channel);
}
