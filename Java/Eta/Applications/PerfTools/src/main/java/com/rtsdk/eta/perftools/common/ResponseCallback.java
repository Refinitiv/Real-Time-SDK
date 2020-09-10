package com.rtsdk.eta.perftools.common;

import com.rtsdk.eta.transport.TransportBuffer;

/** Callback used for processing a response message. */
public interface ResponseCallback
{
	
	/**
	 *  Process the response message.
	 *
	 * @param buffer the buffer
	 */
    public void processResponse(TransportBuffer buffer);
}
