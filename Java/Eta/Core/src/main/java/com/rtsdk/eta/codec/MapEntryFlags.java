package com.rtsdk.eta.codec;

/**
 * Combination of bit values to indicate the presence of any optional {@link MapEntry} content.
 */
public class MapEntryFlags
{
    // MapEntryFlags class cannot be instantiated
    private MapEntryFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) No Map Entry Flags */
    public static final int NONE = 0x00;

    /**
     * (0x01) Indicates that the container entry includes a permData member and
     * also specifies any authorization information for this entry.
     */
    public static final int HAS_PERM_DATA = 0x01;
}
