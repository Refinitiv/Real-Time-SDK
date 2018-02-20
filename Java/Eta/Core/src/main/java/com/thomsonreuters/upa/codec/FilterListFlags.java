package com.thomsonreuters.upa.codec;

/**
 * A combination of bit values to indicate presence of optional
 * {@link FilterList} content.
 * 
 * @see FilterList
 */
public class FilterListFlags
{
    // FilterListFlags class cannot be instantiated
    private FilterListFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) None of the optional flags are set. */
    public static final int NONE = 0x00;

    /**
     * (0x01) Indicates that the Filter Entries in the Filter List contain Permission Data.
     */
    public static final int HAS_PER_ENTRY_PERM_DATA = 0x01;

    /** (0x02) Indicates that the Filter List contains a Total Count Hint. */
    public static final int HAS_TOTAL_COUNT_HINT = 0x02;
}
