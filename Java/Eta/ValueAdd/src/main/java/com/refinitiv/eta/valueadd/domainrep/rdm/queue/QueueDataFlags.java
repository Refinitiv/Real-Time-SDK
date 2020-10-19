package com.refinitiv.eta.valueadd.domainrep.rdm.queue;

/**
 * The queue data flags. Applies to both {@link QueueData} and {@link QueueDataExpired}.
 * 
 * @see QueueData
 * @see QueueDataExpired
 */
public class QueueDataFlags
{
    /** (0x00000) No flags set. */
    public static final int NONE = 0x0000;

    /**
     * (0x0001) Indicates it's possible the data is a duplicate.
     */
    public static final int POSSIBLE_DUPLICATE = 0x0001;
}
