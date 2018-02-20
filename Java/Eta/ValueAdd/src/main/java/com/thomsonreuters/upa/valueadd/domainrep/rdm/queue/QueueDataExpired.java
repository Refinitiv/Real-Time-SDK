package com.thomsonreuters.upa.valueadd.domainrep.rdm.queue;

import com.thomsonreuters.upa.codec.Buffer;

/** The queue data expired message. */
public interface QueueDataExpired extends QueueMsg
{
    
    /**
     * Returns the source name of the local message sender.
     * Use to retrieve and set the source name.
     *
     * @return the buffer
     */
    public Buffer sourceName();

    /**
     * Returns the destination name of the remote message receiver.
     * Use to retrieve and set the destination name.
     *
     * @return the buffer
     */
    public Buffer destName();
    
    /**
     * Sets the identifier of the queue data expired message.
     *
     * @param identifier the identifier
     */
    public void identifier(long identifier);
    
    /**
     * Returns the identifier of the queue data expired message.
     *
     * @return the long
     */
    public long identifier();

    /**
     * Sets the encoded data body of the queue data expired message.
     *
     * @param data the data
     */
    public void encodedDataBody(Buffer data);

    /**
     * Returns the encoded data body of the queue data expired message.
     *
     * @return the buffer
     */
    public Buffer encodedDataBody();
    
    /**
     * Sets the undeliverableCode of the queue data expired message.
     *
     * @param undeliverableCode the undeliverable code
     */
    public void undeliverableCode(int undeliverableCode);

    /**
     * Returns the undeliverableCode of the queue data expired message.
     *
     * @return the int
     */
    public int undeliverableCode();
    
    /**
     * Sets the container type of the queue data expired message.
     *
     * @param containerType the container type
     */
    public void containerType(int containerType);
    
    /**
     * Returns the container type of the queue data expired message.
     *
     * @return the int
     */
    public int containerType();
    
    /**
     * Sets the flags of the queue data expired message.
     *
     * @param flags the flags
     * @see QueueDataFlags
     */
    public void flags(int flags);
    
    /**
     * Returns the flags of the queue data expired message.
     *
     * @return the int
     * @see QueueDataFlags
     */
    public int flags();

    /**
     * Sets the number of messages remaining in the queue for this stream.
     *
     * @param queueDepth the queue depth
     */
    public void queueDepth(int queueDepth);
    
    /**
     * Returns the number of messages remaining in the queue for this stream.
     *
     * @return the int
     */
    public int queueDepth();
    
    /**
     * Checks if possible duplicate flag is set.
     * 
     * @return true - if possible duplicate is set, false - if not.
     */
    public boolean checkPossibleDuplicate();
}
