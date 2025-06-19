/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.common;

/**
 * Status flags used by the ETA Java Generic Consumer and Provider applications.
 */
public class GenericResponseStatusFlags
{
    // GenericResponseStatusFlags class cannot be instantiated
    private GenericResponseStatusFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) Generic response successful. */
    public static final int SUCCESS = 0x00;

    /** (0x01) Generic response failure. */
    public static final int FAILURE = 0x01;
}
