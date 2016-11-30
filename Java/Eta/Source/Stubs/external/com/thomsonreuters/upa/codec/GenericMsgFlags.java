package com.thomsonreuters.upa.codec;

/**
 * Specifies a combination of bit values indicating special behaviors and the presence of optional content.
 * 
 * @see GenericMsg
 */
public class GenericMsgFlags
{
    /**
     * This class is not instantiated
     */
    private GenericMsgFlags()
    {
        throw new AssertionError();
    }

    /** (0x000) No Flags set */
    public static final int NONE = 0x000;

    /** (0x001) Indicates the presence of extended header */
    public static final int HAS_EXTENDED_HEADER = 0x001;

    /** (0x002) Indicates the presence of permission expression */
    public static final int HAS_PERM_DATA = 0x002;

    /**
     * (0x004) Indicates the presence of a populated {@link MsgKey}. Use of a
     * {@link MsgKey} differentiates a generic message from the msgKey
     * information specified for other messages within the stream. Contents and
     * semantics associated with a {@link GenericMsg#msgKey()} should be
     * defined by the domain model specification that employs them.
     */
    public static final int HAS_MSG_KEY = 0x004;

    /** (0x008) Indicates the presence of sequence number */
    public static final int HAS_SEQ_NUM = 0x008;

    /**
     * (0x010) Indicates that the message is the final part of a
     * {@link GenericMsg}. This flag should be set on:
     * <ul>
     * <li>Single-part generic messages (i.e., an atomic generic message).</li>
     * <li>The last message (final part) in a multi-part generic message</li>
     * </ul>
     */
    public static final int MESSAGE_COMPLETE = 0x010;

    /** (0x020) Indicates the presence of the secondary sequence number */
    public static final int HAS_SECONDARY_SEQ_NUM = 0x020;

    /** (0x040) Indicates the presence of the partNum */
    public static final int HAS_PART_NUM = 0x040;
}
