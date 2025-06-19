/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

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
