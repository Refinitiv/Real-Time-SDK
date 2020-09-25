package com.refinitiv.eta.codec;

/**
 * Specifies a combination of bit values indicating special behaviors and the presence of optional content.
 * 
 * @see StatusMsg
 */
public class StatusMsgFlags
{
    /**
     * This class is not instantiated
     */
    private StatusMsgFlags()
    {
        throw new AssertionError();
    }

    /** (0x000) No Flags set */
    public static final int NONE = 0x000;

    /** (0x001) Indicates that Status Message has Extended Header */
    public static final int HAS_EXTENDED_HEADER = 0x001;

    /**
     * (0x002) Indicates that Status Message has Permission Expression.
     * When present, the message might be changing the stream's permission information.
     */
    public static final int HAS_PERM_DATA = 0x002;

    /** (0x008) Indicates that Status Message has a msgKey */
    public static final int HAS_MSG_KEY = 0x008;

    /**
     * (0x010) Indicates that Status Message has Group Id.
     * When present, the message might be changing the stream's groupId.
     */
    public static final int HAS_GROUP_ID = 0x010;

    /**
     * (0x020) Indicates that Status Message has State. If state information is
     * not present, the message might be changing the stream's permission information or groupId.
     */
    public static final int HAS_STATE = 0x020;

    /**
     * (0x040) Indicates that the application should clear stored header or
     * payload information associated with the stream.
     * This can happen if some portion of data is invalid.
     */
    public static final int CLEAR_CACHE = 0x040;

    /**
     * (0x080) Acknowledges the establishment of a private stream, or when
     * combined with a streamState value of {@link StreamStates#REDIRECTED},
     * indicates that a stream can be opened only as private.
     */
    public static final int PRIVATE_STREAM = 0x080;

    /**
     * (0x100) Indicates that this data was posted by the user with this
     * identifying information
     */
    public static final int HAS_POST_USER_INFO = 0x100;

    /* (0x200) reserved */
    
    /**
     * (0x400) Acknowledges the establishment of a qualified stream.
     */
    public static final int QUALIFIED_STREAM = 0x400;
}
