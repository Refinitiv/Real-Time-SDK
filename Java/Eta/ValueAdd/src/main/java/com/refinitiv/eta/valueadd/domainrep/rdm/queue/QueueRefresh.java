package com.refinitiv.eta.valueadd.domainrep.rdm.queue;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.State;

/** The queue refresh message. */
public interface QueueRefresh extends QueueMsg
{
    
    /**
     * Returns the source name of the queue refresh message. Use to
     * retrieve and set the source name of the queue refresh message.
     *
     * @return the buffer
     */
    public Buffer sourceName();

    /**
     * Returns the state of the queue refresh message. Use to
     * retrieve and set the state of the queue refresh message.
     *
     * @return the state
     */
    public State state();
    
    /**
     * Returns the number of messages remaining in the queue for this stream.
     *
     * @return the int
     */
    public int queueDepth();
}
