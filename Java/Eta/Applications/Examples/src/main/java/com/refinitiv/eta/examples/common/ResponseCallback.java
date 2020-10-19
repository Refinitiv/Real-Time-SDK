package com.refinitiv.eta.examples.common;

import com.refinitiv.eta.transport.TransportBuffer;

/** Callback used for processing a response message. */
public interface ResponseCallback
{
    public void processResponse(ChannelSession chnl, TransportBuffer buffer);
}
