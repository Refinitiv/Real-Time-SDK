package com.thomsonreuters.upa.valueadd.domainrep.rdm.queue;

import com.thomsonreuters.upa.codec.Buffer;

/** The queue data expired message. */
public interface QueueDataExpired extends QueueMsg
{
    /**
     * Returns the source name of the local message sender.
     * Use to retrieve and set the source name.
     */
    public Buffer sourceName();

    /**
     * Returns the destination name of the remote message receiver.
     * Use to retrieve and set the destination name.
     */
    public Buffer destName();
    
    /**
     * Sets the identifier of the queue data expired message.
     */
    public void identifier(long identifier);
    
    /**
     * Returns the identifier of the queue data expired message.
     */
    public long identifier();

    /**
     * Sets the encoded data body of the queue data expired message.
     */
    public void encodedDataBody(Buffer data);

    /**
     * Returns the encoded data body of the queue data expired message.
     */
    public Buffer encodedDataBody();
    
    /**
     * Sets the undeliverableCode of the queue data expired message.
     */
    public void undeliverableCode(int undeliverableCode);

    /**
     * Returns the undeliverableCode of the queue data expired message.
     */
    public int undeliverableCode();
    
    /**
     * Sets the container type of the queue data expired message.
     */
    public void containerType(int containerType);
    
    /**
     * Returns the container type of the queue data expired message.
     */
    public int containerType();
    
    /**
     * Sets the flags of the queue data expired message.
     * 
     * @see QueueDataFlags
     */
    public void flags(int flags);
    
    /**
     * Returns the flags of the queue data expired message.
     * 
     * @see QueueDataFlags
     */
    public int flags();

    /**
     * Sets the number of messages remaining in the queue for this stream.
     */
    public void queueDepth(int queueDepth);
    
    /**
     * Returns the number of messages remaining in the queue for this stream.
     */
    public int queueDepth();
    
    /**
     * Checks if possible duplicate flag is set.
     * 
     * @return true - if possible duplicate is set, false - if not.
     */
    public boolean checkPossibleDuplicate();
}
