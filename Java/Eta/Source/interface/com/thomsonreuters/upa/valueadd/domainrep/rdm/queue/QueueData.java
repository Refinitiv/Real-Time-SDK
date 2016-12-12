package com.thomsonreuters.upa.valueadd.domainrep.rdm.queue;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.EncodeIterator;

/** The queue data message. */
public interface QueueData extends QueueMsg
{   
    /**
     * Returns the source name of the local message sender.
     * Use to retrieve and set the source name.
     */      
    public Buffer sourceName();
    
    /**
     * Sets the source name of the queue data message.
     */
    public void sourceName(Buffer sourceName);
    
    /**
     * Returns the destination name of the remote message receiver.
     * Use to retrieve and set the destination name.
     */
    public Buffer destName();
    
    /**
     * Sets the destination name of the remote message receiver.
     */
    public void destName(Buffer destName);
    
    /**
     * Sets the identifier of the queue data message.
     */
    public void identifier(long identifier);
    
    /**
     * Returns the identifier of the queue data message.
     */
    public long identifier();

    /**
     * Sets the timeout (in milliseconds) at which the message should expire instead
     * of being delivered. Can also be set to {@link QueueDataTimeoutCode#IMMEDIATE}
     * or {@link QueueDataTimeoutCode#INFINITE}.
     * 
     * @see QueueDataTimeoutCode
     */
    public void timeout(long timeoutMsec);
    
    /**
     * Returns the timeout (in milliseconds) at which the message should expire instead
     * of being delivered. Can also be set to {@link QueueDataTimeoutCode#IMMEDIATE}
     * or {@link QueueDataTimeoutCode#INFINITE}.
     * 
     * @see QueueDataTimeoutCode
     */
    public long timeout();
    
    /**
     * Sets the number of messages remaining in the queue for this stream.
     */
    public void queueDepth(int queueDepth);
    
    /**
     * Returns the number of messages remaining in the queue for this stream.
     */
    public int queueDepth();
    
    /**
     * Initialize the encoding of a queue data message.
     * This is to be done before encoding any message payload.
     * 
     * @param eIter The Encode Iterator
     * 
     * @return UPA return value
     */
    public int encodeInit(EncodeIterator eIter);
    
    /**
     * Complete the encoding of a queue data message.
     * This is to be done after encoding any message payload.
     * 
     * @param eIter The Encode Iterator
     * @param success If true - successfully complete the message, if false - remove
     *            the message from the buffer.
     * 
     * @return UPA return value
     */
    public int encodeComplete(EncodeIterator eIter, boolean success);

    /**
     * Sets the encoded data body of the queue data message.
     */
    public void encodedDataBody(Buffer data);

    /**
     * Returns the encoded data body of the queue data message.
     */
    public Buffer encodedDataBody();
    
    /**
     * Sets the container type of the queue data message.
     */
    public void containerType(int containerType);
    
    /**
     * Returns the container type of the queue data message.
     */
    public int containerType();
    
    /**
     * Sets the flags of the queue data message.
     * 
     * @see QueueDataFlags
     */
    public void flags(int flags);
    
    /**
     * Returns the flags of the queue data message.
     * 
     * @see QueueDataFlags
     */
    public int flags();

    /**
     * Checks if possible duplicate flag is set.
     * 
     * @return true - if possible duplicate is set, false - if not.
     */
    public boolean checkPossibleDuplicate();
}
