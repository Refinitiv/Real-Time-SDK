package com.refinitiv.eta.codec;

/**
 * A combination of bit values that indicate special behaviors and the presence
 * of optional {@link RefreshMsg} content.
 * 
 * @see RefreshMsg
 */
public class RefreshMsgFlags
{
    /**
     * This class is not instantiated
     */
    private RefreshMsgFlags()
    {
        throw new AssertionError();
    }

    /** (0x0000) No Flags set */
    public static final int NONE = 0x0000;

    /** (0x0001) Indicates the presence of the Extended Header */
    public static final int HAS_EXTENDED_HEADER = 0x0001;

    /** (0x0002) Indicates the presence of Permission Expression */
    public static final int HAS_PERM_DATA = 0x0002;

    /**
     * (0x0008) Indicates that the Refresh Message contains a populated msgKey.
     * This can associate a request with a refresh or identify an item sent from
     * an NIP application.
     */
    public static final int HAS_MSG_KEY = 0x0008;

    /** (0x0010) Indicates the presence of Sequence Number */
    public static final int HAS_SEQ_NUM = 0x0010;

    /**
     * (0x0020) Indicates that the refresh is sent as a response to a request,
     * referred to as a solicited refresh. A refresh sent to inform a consumer
     * of an upstream change in information (i.e., an unsolicited refresh) must
     * not include this flag.
     */
    public static final int SOLICITED = 0x0020;

    /**
     * (0x0040) Indicates that the message is the final part of the
     * {@link RefreshMsg}. This flag value should be set when:
     * <ul>
     * <li>The message is a single-part refresh (i.e., atomic refresh).</li>
     * <li>The message is the final part of a multi-part refresh.</li>
     * </ul>
     */
    public static final int REFRESH_COMPLETE = 0x0040;

    /** (0x0080) Indicates the presence of Qos */
    public static final int HAS_QOS = 0x0080;

    /**
     * (0x0100) Indicates that the stream's stored payload information should be
     * cleared. This might occur if some portion of data is known to be invalid.
     */
    public static final int CLEAR_CACHE = 0x0100;

    /**
     * (0x0200) Indicates that the message's payload information should not be
     * cached. This flag value only applies to the message on which it is present.
     */
    public static final int DO_NOT_CACHE = 0x0200;

    /**
     * (0x0400) Acknowledges the initial establishment of a private stream or,
     * when combined with a streamState value of {@link StreamStates#REDIRECTED},
     * indicates that a stream can only be opened as private.
     */
    public static final int PRIVATE_STREAM = 0x0400;

    /**
     * (0x0800) Indicates that this message includes postUserInfo, implying that
     * this {@link RefreshMsg} was posted by the user described in postUserInfo.
     */
    public static final int HAS_POST_USER_INFO = 0x0800;

    /** (0x1000) Indicates the presence of a Part Number */
    public static final int HAS_PART_NUM = 0x1000;

    /* (0x2000) reserved */
    
    /**
     * (0x4000) Acknowledges the initial establishment of a qualified stream.
     */
    public static final int QUALIFIED_STREAM = 0x4000;
}
