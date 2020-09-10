package com.rtsdk.eta.codec;

/**
 * Combination of bit values that indicate whether optional, element-list content is present.
 * 
 * @see ElementList
 */
public class ElementListFlags
{
    // ElementListFlags class cannot be instantiated
    private ElementListFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) None of the optional flags are set. */
    public static final int NONE = 0x00;

    /**
     * (0x01) Indicates the presence of has element list number.
     * This member is provided as part of the initial refresh message on a stream or on the
     * first refresh message after a CLEAR_CACHE command.
     */
    public static final int HAS_ELEMENT_LIST_INFO = 0x01;

    /**
     * (0x02) Indicates that {@link ElementList} contains set-defined data.
     * If both standard and set-defined data are present in this
     * {@link ElementList}, this value can be set in addition to {@link #HAS_STANDARD_DATA}.
     * If no entries are present in the {@link ElementList}, do not set this flag value.
     */
    public static final int HAS_SET_DATA = 0x02;

    /**
     * (0x04) Indicates the presence of a setId and determines the set
     * definition to use when encoding or decoding set data on this {@link ElementList}.
     */
    public static final int HAS_SET_ID = 0x04;

    /**
     * (0x08) Indicates that the {@link ElementList} contains standard element
     * name, dataType, value-encoded data. You can set this value in addition to
     * {@link #HAS_SET_DATA} if both standard and set-defined data are present
     * in this {@link ElementList}. If no entries are present in the
     * {@link ElementList}, do not set this flag value.
     */
    public static final int HAS_STANDARD_DATA = 0x08;
}
