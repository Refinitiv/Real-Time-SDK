package com.refinitiv.eta.valueadd.domainrep.rdm.queue;

import com.refinitiv.eta.codec.Buffer;

/** The queue request message. Used by an OMM Consumer to connect to a QueueProvider. */
public interface QueueRequest extends QueueMsg
{
    
    /**
     * Returns the source name of the queue request message. Use to
     * retrieve and set the source name of the queue request message.
     *
     * @return the buffer
     */
    public Buffer sourceName();
    
    /**
     * Sets the source name of the queue request message. Use to
     *
     * @param sourceName the source name
     */   
    public void sourceName(Buffer sourceName);	
	    
}
