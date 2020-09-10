package com.rtsdk.eta.codec;

/**
 * Combination of bit values that indicate whether optional {@link Vector} content is present.
 * 
 * @see Vector
 */
public class VectorFlags
{
    // VectorFlags class cannot be instantiated
    private VectorFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) No flags */
    public static final int NONE = 0x00;

    /**
     * (0x01) Indicates that the {@link Vector} contains local set definition
     * information. Local set definitions correspond to data contained in this
     * {@link Vector}'s entries and are used for encoding or decoding.
     */
    public static final int HAS_SET_DEFS = 0x01;

    /**
     * (0x02) Indicates that the {@link Vector} contains summary data.
     * <ul>
     * <li>If this flag is set while encoding, summary data must be provided by
     * encoding or populating encodedSummaryData with pre-encoded data.</li>
     * <li>If this flag is set while decoding, summary data is contained as part
     * of {@link Vector} and the user can choose whether to decode it.</li>
     * </ul>
     */
    public static final int HAS_SUMMARY_DATA = 0x02;

    /**
     * (0x04) Indicates that permission information is included with some vector
     * entries. The {@link Vector} encoding functionality sets this flag value
     * on the user's behalf if an entry is encoded with its own permData. A
     * decoding application can check this flag to determine whether a contained
     * entry has permData and is often useful for fan out devices (if an entry
     * does not have permData, the fan out device can likely pass on data and
     * not worry about special permissioning for the entry). Each entry also
     * indicates the presence of permission data via the use of
     * {@link VectorEntryFlags#HAS_PERM_DATA}.
     */
    public static final int HAS_PER_ENTRY_PERM_DATA = 0x04;

    /**
     * (0x08) Indicates that the totalCountHint member is present.
     * totalCountHint can provide an approximation of the total number of
     * entries sent across all vectors on all parts of the refresh message. Such
     * information is useful in determining the amount of resources to allocate
     * for caching or displaying all expected entries.
     */
    public static final int HAS_TOTAL_COUNT_HINT = 0x08;

    /**
     * (0x10) Indicates that the {@link Vector} may leverage sortable action
     * types. If a {@link Vector} is sortable, all components must properly
     * handle changing index values based on insert and delete actions. If a
     * component does not properly handle these action types, it can result in
     * the corruption of the {@link Vector}'s contents.
     */
    public static final int SUPPORTS_SORTING = 0x10;
}
