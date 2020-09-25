package com.refinitiv.eta.codec;

/**
 * 
 * Combination of bit values that indicate whether optional {@link VectorEntry} content is present.
 * 
 * @see VectorEntry
 */
public class VectorEntryFlags
{
    // VectorEntryFlags class cannot be instantiated
    private VectorEntryFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) No Vector Entry Flags */
    public static final int NONE = 0x00;

    /**
     * (0x01) Indicates the presence of the Permission Expression in this
     * container entry and indicates authorization information for this entry.
     */
    public static final int HAS_PERM_DATA = 0x01;
}
