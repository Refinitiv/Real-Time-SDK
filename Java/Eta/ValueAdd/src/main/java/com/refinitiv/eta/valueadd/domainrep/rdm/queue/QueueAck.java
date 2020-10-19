package com.refinitiv.eta.valueadd.domainrep.rdm.queue;

import com.refinitiv.eta.codec.Buffer;

/** The queue acknowledgment message. */
public interface QueueAck extends QueueMsg
{
    /**
     * Sets the identifier of the queue acknowledgment message.
     * @param identifier of the queue acknowledgment message
     */
    public void identifier(long identifier);
    
    /**
     * Returns the identifier of the queue acknowledgment message.
     * @return the identifier of the queue acknowledgment message
     */
    public long identifier();
    
    /**
     * Returns the source name of the queue refresh message. Use to
     * retrieve and set the source name of the queue refresh message.
     * @return source name of the queue refresh message
     */
    public Buffer sourceName();
    
    /**
     * Sets the source name of the queue refresh message. Use to
     * @param sourceName of the queue refresh message
     */    
    public void sourceName(Buffer sourceName);

    /**
     * Returns the destination name of the remote message receiver.
     * Use to retrieve and set the destination name.
     * @return the destination name of the remote message receiver
     */
    public Buffer destName();
    
    /**
     * Sets the destination name of the queue refresh message. Use to
     * @param destName name of the queue refresh message
     */      
    public void destName(Buffer destName);
        
}
