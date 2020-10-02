package com.refinitiv.eta.transport;

/**
 * ETA Write Priorities passed into the {@link Channel#write(TransportBuffer, WriteArgs, Error)} method call.
 * 
 * @see Channel
 */
public class WritePriorities
{
    // WritePriorities class cannot be instantiated
    private WritePriorities()
    {
        throw new AssertionError();
    }

    /** Assigns message to the high priority flush, if not directly written to the socket. */
    public static final int HIGH = 0;
	
    /** Assigns message to the medium priority flush, if not directly written to the socket. */
    public static final int MEDIUM = 1;
	
    /** Assigns message to the low priority flush, if not directly written to the socket. */
    public static final int LOW = 2;
}
