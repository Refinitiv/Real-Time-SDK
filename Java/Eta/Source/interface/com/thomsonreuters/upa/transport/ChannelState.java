package com.thomsonreuters.upa.transport;

/**
 * UPA Channel states.
 */
public class ChannelState
{
    // ChannelState class cannot be instantiated
    private ChannelState()
    {
        throw new AssertionError();
    }

    /** 
     * Channel has been CLOSED. 
     * Channel is set in this state when any socket related operation failed
     * because the far end connection has been closed.
     */
    public static final int CLOSED = -1;

    /**
     * Indicates that a {@link Channel} is inactive. This channel cannot be used.
     * This state typically occurs after a channel is closed by the user.
     */
    public static final int INACTIVE = 0;

    /**
     * Indicates that a {@link Channel} requires additional initialization.
     * This initialization is typically additional connection handshake messages
     * that need to be exchanged. When using blocking I/O, a {@link Channel} is
     * typically active when it is returned and no additional initialization is
     * required by the user.
     */
    public static final int INITIALIZING = 1;

    /**
     * Indicates that a {@link Channel} is active. This channel can perform any
     * connection related actions, such as reading or writing.
     */
    public static final int ACTIVE = 2;

    /**
     * Returns a string representation of the specified state
     */
    public static final String toString(int state)
    {
        final String description;

        switch (state)
        {
            case CLOSED:
                description = "CLOSED";
                break;
            case INACTIVE:
                description = "INACTIVE";
                break;
            case INITIALIZING:
                description = "INITIALIZING";
                break;
            case ACTIVE:
                description = "ACTIVE";
                break;
            default:
                description = Integer.toString(state);
                break;
        }

        return description;
    }
}
