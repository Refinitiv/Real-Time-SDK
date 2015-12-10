package com.thomsonreuters.upa.examples.common;

import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.TransportBuffer;

/**
 * Callback method used by {@link ProviderSession#read(Channel, com.thomsonreuters.upa.transport.Error, ReceivedMsgCallback)} to process client request.
 */
public interface ReceivedMsgCallback
{
    public void processReceivedMsg(Channel channel, TransportBuffer msgBuf);

    public void processChannelClose(Channel channel);
}
