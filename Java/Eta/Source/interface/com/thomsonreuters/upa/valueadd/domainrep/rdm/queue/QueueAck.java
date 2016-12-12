package com.thomsonreuters.upa.valueadd.domainrep.rdm.queue;

import com.thomsonreuters.upa.codec.Buffer;

/** The queue acknowledgment message. */
public interface QueueAck extends QueueMsg
{
    /**
     * Sets the identifier of the queue acknowledgment message.
     */
    public void identifier(long identifier);
    
    /**
     * Returns the identifier of the queue acknowledgment message.
     */
    public long identifier();
    
    /**
     * Returns the source name of the queue refresh message. Use to
     * retrieve and set the source name of the queue refresh message.
     */
    public Buffer sourceName();
    
    /**
     * Sets the source name of the queue refresh message. Use to
     */    
    public void sourceName(Buffer sourceName);

    /**
     * Returns the destination name of the remote message receiver.
     * Use to retrieve and set the destination name.
     */
    public Buffer destName();
    
    /**
     * Sets the destination name of the queue refresh message. Use to
     */      
    public void destName(Buffer destName);
        
}
