package com.thomsonreuters.upa.codec;

/**
 * A combination of bit values that indicates the presence of optional
 * {@link Series} content.
 * 
 * @see Series
 */
public class SeriesFlags
{
    // SeriesFlags class cannot be instantiated
    private SeriesFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) None of the optional flags are present. */
    public static final int NONE = 0x00;

    /**
     * (0x01) Indicates that the {@link Series} contains local set definition
     * information. Local set definitions correspond to data contained in this
     * Series's entries and encode or decode their contents.
     */
    public static final int HAS_SET_DEFS = 0x01;

    /**
     * (0x02) Indicates that the {@link Series} contains summary data.
     * <ul>
     * <li>If set while encoding, summary data must be provided by encoding or
     * populating encodedSummaryData with pre-encoded information.</li>
     * <li>If set while decoding, summary data is contained as part of
     * {@link Series} and the user can choose to decode it.</li>
     * </ul>
     */
    public static final int HAS_SUMMARY_DATA = 0x02;

    /**
     * (0x04) Indicates the presence of the totalCountHint member, which can
     * provide an approximation of the total number of entries sent across maps
     * on all parts of the refresh message. Such information is useful when
     * determining resource allocation for caching or displaying all expected
     * entries.
     */
    public static final int HAS_TOTAL_COUNT_HINT = 0x04;
}
