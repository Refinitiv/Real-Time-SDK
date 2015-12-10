package com.thomsonreuters.upa.examples.common;

import com.thomsonreuters.upa.transport.TransportBuffer;

/** Callback used for processing a response message. */
public interface ResponseCallback
{
    public void processResponse(ChannelSession chnl, TransportBuffer buffer);
}
