/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/** 
 * Enumerated Marketfeed types.
 */
public class MfFieldTypes
{
    // MfFieldTypes class cannot be instantiated
    private MfFieldTypes()
    {
        throw new AssertionError();
    }

    public static final int NONE            = -1;
    public static final int TIME_SECONDS    = 0;
    public static final int INTEGER         = 1;
    public static final int DATE            = 3;
    public static final int PRICE           = 4;
    public static final int ALPHANUMERIC    = 5;
    public static final int ENUMERATED      = 6;
    public static final int TIME            = 7;
    public static final int BINARY          = 8;
}
