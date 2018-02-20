package com.thomsonreuters.upa.codec;

/**
 * Specifies a combination of bit values indicating special behaviors and the
 * presence of optional content for an {@link AckMsg}.
 * 
 * @see AckMsg
 */
public class AckMsgFlags
{
    /**
     * This class is not instantiated
     */
    private AckMsgFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) No Flags set */
    public static final int NONE = 0x00;

    /** (0x01) Indicates that Ack Message has Extended Header. */
    public static final int HAS_EXTENDED_HEADER = 0x01;

    /** (0x02) Indicates that Ack Message Has Text */
    public static final int HAS_TEXT = 0x02;

    /**
     * (0x04) Indicates that Ack message acknowledges private stream establishment.
     */
    public static final int PRIVATE_STREAM = 0x04;

    /** (0x08) Indicates that Ack Message contains a sequence number. */
    public static final int HAS_SEQ_NUM = 0x08;

    /**
     * (0x10) Indicates that Ack Message contains a msgKey. When present, this
     * is typically populated to match the information being acknowledged.
     */
    public static final int HAS_MSG_KEY = 0x10;

    /** (0x20) Indicates that Ack Message contains a NAK Code {@link NakCodes}. */
    public static final int HAS_NAK_CODE = 0x20;

    /**
     * (0x40) Indicates that Ack message acknowledges qualified stream establishment.
     */
    public static final int QUALIFIED_STREAM = 0x40;
}
