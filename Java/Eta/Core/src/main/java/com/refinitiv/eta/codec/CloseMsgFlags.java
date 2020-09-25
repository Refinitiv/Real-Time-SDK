package com.refinitiv.eta.codec;

/**
 * Specifies a combination of bit values indicating special behaviors and the
 * presence of optional content for a {@link CloseMsg}.
 * 
 * @see CloseMsg
 */
public class CloseMsgFlags
{
    /**
     * This class is not instantiated
     */
    private CloseMsgFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) No Flags set */
    public static final int NONE = 0x00;

    /** (0x01) Indicates that Close Message has Extended Header */
    public static final int HAS_EXTENDED_HEADER = 0x01;

    /**
     * (0x02) If present, the consumer wants the provider to send an
     * {@link AckMsg} to indicate that the {@link CloseMsg} has been processed
     * properly and the stream is properly closed. This functionality might not
     * be available with some components; for details, refer to the component's documentation.
     */
    public static final int ACK = 0x02;
}
