/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Combination of bit values to indicate the presence of optional {@link Map} content.
 * 
 * @see Map
 */
public class MapFlags
{
    // MapFlags class cannot be instantiated
    private MapFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) No map flags. */
    public static final int NONE = 0x00;

    /**
     * (0x01) Indicates that Map contains local set definition information.
     * Local set definitions; correspond to data contained within this Map's
     * entries and are used for encoding or decoding their contents.
     */
    public static final int HAS_SET_DEFS = 0x01;

    /**
     * (0x02) Indicates that Map contains summary data. If this flag is set
     * while encoding, summary data must be provided by encoding or populating
     * encodedSummaryData with pre-encoded information. If this flag is set
     * while decoding, summary data is contained as part of Map and the user can
     * choose whether to decode it.
     */
    public static final int HAS_SUMMARY_DATA = 0x02;

    /**
     * (0x04) Indicates that permission information is included with some map
     * entries. The Map encoding functionality sets this flag value on the
     * user's behalf if any entry is encoded with its own permData. A decoding
     * application can check this flag to determine if any contained entry has
     * permData, often useful for fan out devices (if an entry does not have
     * permData, the fan out device can likely pass on data and not worry about
     * special permissioning for the entry). Each entry will also indicate the
     * presence of permission data via the use of
     * {@link MapEntryFlags#HAS_PERM_DATA}.
     */
    public static final int HAS_PER_ENTRY_PERM_DATA = 0x04;

    /**
     * Indicates presence of the totalCountHint member. This member can provide
     * an approximation of the total number of entries sent across all maps on
     * all parts of the refresh message. This information is useful when
     * determining the amount of resources to allocate for caching or displaying
     * all expected entries.
     */
    public static final int HAS_TOTAL_COUNT_HINT = 0x08;

    /**
     * Indicates the presence of the keyFieldId member. keyFieldId should be
     * provided if the key information is also a field that would be contained
     * in the entry payload. This optimization allows for keyFieldId to be
     * included once instead of in every entry's payload.
     */
    public static final int HAS_KEY_FIELD_ID = 0x10;
}
