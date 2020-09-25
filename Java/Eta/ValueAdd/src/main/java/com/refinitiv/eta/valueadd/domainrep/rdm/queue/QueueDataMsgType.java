package com.refinitiv.eta.valueadd.domainrep.rdm.queue;

/**
 * Type used with Queue Data Messages.
 * 
 * @see QueueData
 */
public class QueueDataMsgType
{
    /** Queue data message initialization value. This should not be used by the application or returned to the application. */
    public static final int INIT = 0;
    
    /** Indicates a normal data message. */
    public static final int MSG_DATA = 1; 

    /** Indicates an undeliverable data message. */
    public static final int MSG_UNDELIVERABLE = 2;
}
