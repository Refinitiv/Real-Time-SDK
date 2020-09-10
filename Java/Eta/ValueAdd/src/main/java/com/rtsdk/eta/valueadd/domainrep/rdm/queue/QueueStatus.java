package com.rtsdk.eta.valueadd.domainrep.rdm.queue;

import com.rtsdk.eta.codec.State;

/** The queue status message. */
public interface QueueStatus extends QueueMsg
{
    
    /**
     * The Queue Status flags. Populated by {@link QueueStatusFlags}.
     *
     * @param flags the flags
     */
    public void flags(int flags);

    /**
     * The Queue Status flags. Populated by {@link QueueStatusFlags}.
     * 
     * @return flags
     */
    public int flags();
    
    /**
     * Checks the presence of state field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if state field is present, false - if not.
     */
    public boolean checkHasState();

    /**
     * Applies state presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasState();

    /**
     * Returns the state of the queue status message. Use to
     * retrieve and set the state of the queue status message.
     *
     * @return the state
     */
    public State state();
}
