/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

class RwfDataConstants
{
    // RwfDataConstants class cannot be instantiated
    private RwfDataConstants()
    {
        throw new AssertionError();
    }

    public static final byte MAJOR_VERSION_1 = 14;
    public static final byte MINOR_VERSION_1 = 1;
}
