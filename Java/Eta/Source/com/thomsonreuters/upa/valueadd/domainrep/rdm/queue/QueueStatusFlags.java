package com.thomsonreuters.upa.valueadd.domainrep.rdm.queue;

/**
 * The queue status flags.
 * 
 * @see QueueStatus
 */
public class QueueStatusFlags
{
    /** (0x00) No flags set. */
    public static final int NONE = 0x00;
   
    /** (0x01) Indicates presence of the state member. */
    public static final int HAS_STATE = 0x01;
}
