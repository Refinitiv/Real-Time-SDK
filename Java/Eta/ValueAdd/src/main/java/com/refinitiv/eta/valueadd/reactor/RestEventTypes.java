/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * RestEventTypes used with RestEvent.
 */
class RestEventTypes
{
    public static final int COMPLETED = 0;
    public static final int FAILED = 1;
    public static final int CANCELLED = 2;
    public static final int STOPPED = 3;

    public static String toString(int type)
    {
        switch (type)
        {
            case 0:
                return "RestEventTypes.COMPLETED";
            case 1:
                return "RestEventTypes.FAILED";
            case 2:
                return "RestEventTypes.CANCELLED";
            case 3:
            	return "RestEventTypes.STOPPED";
            default:
                return "RestEventTypes " + type + " - undefined.";
        }
    }
}
