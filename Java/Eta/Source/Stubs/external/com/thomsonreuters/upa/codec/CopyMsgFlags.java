package com.thomsonreuters.upa.codec;

/**
 * Specifies a combination of bit values indicating how {@link Msg} is copied.
 * 
 * @see Msg#copy(Msg, int)
 */
public class CopyMsgFlags
{
    /**
     * This class is not instantiated
     */
    private CopyMsgFlags()
    {
        throw new AssertionError();
    }

    /** (0x000) No Flags set; no sub-structs will be copied */
    public static final int NONE = 0x000;

    /** (0x001) State test will be copied */
    public static final int STATE_TEXT = 0x001;

    /** (0x002) Perm exp will be copied */
    public static final int PERM_DATA = 0x002;

    /** (0x004) Key name will be copied */
    public static final int KEY_NAME = 0x004;

    /** (0x008) Key attrib will be copied */
    public static final int KEY_ATTRIB = 0x008;

    /** (0x00C) Entire key will be copied */
    public static final int KEY = 0x00C;

    /** (0x010) Extended header will be copied */
    public static final int EXTENDED_HEADER = 0x010;

    /** (0x020) Data body will be copied */
    public static final int DATA_BODY = 0x020;

    /** (0x040) Encoded message buffer will be copied */
    public static final int MSG_BUFFER = 0x040;

    /** (0x080) Group Id will be copied */
    public static final int GROUP_ID = 0x080;

    /** (0x100) Nak Text will be copied */
    public static final int NAK_TEXT = 0x100;

    /** (0xFFF) Everything will be copied */
    public static final int ALL_FLAGS = 0xFFF;
}
