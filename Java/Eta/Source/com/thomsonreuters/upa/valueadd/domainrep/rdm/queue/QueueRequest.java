package com.thomsonreuters.upa.valueadd.domainrep.rdm.queue;

import com.thomsonreuters.upa.codec.Buffer;

/** The queue request message. Used by an OMM Consumer to connect to a QueueProvider. */
public interface QueueRequest extends QueueMsg
{
    /**
     * Returns the source name of the queue request message. Use to
     * retrieve and set the source name of the queue request message.
     */
    public Buffer sourceName();
    
    /**
     * Sets the source name of the queue request message. Use to
     */   
    public void sourceName(Buffer sourceName);	
	    
}
