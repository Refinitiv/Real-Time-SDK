/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

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
