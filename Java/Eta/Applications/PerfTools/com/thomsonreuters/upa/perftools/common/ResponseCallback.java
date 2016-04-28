package com.thomsonreuters.upa.perftools.common;

import com.thomsonreuters.upa.transport.TransportBuffer;

/** Callback used for processing a response message. */
public interface ResponseCallback
{
	/** Process the response message. */
    public void processResponse(TransportBuffer buffer);
}
