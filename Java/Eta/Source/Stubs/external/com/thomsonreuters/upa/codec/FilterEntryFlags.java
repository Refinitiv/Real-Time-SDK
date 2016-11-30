package com.thomsonreuters.upa.codec;

/**
 * A combination of bit values that indicate the presence of optional {@link FilterEntry} content.
 * 
 * @see FilterEntry
 */
public class FilterEntryFlags
{
    // FilterEntryFlags class cannot be instantiated
    private FilterEntryFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) No filter item flags */
    public static final int NONE = 0x00;

    /**
     * (0x01) Indicates the presence of permData in this container entry and
     * indicates authorization information for this entry.
     */
    public static final int HAS_PERM_DATA = 0x01;

    /**
     * (0x02) Indicates the presence of containerType in this entry. This flag
     * is used when the entry differs from the specified
     * {@link FilterList#containerType()}.
     */
    public static final int HAS_CONTAINER_TYPE = 0x02;
}
