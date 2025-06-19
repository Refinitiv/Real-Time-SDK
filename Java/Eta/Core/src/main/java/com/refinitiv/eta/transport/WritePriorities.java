/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * ETA Write Priorities passed into the {@link Channel#write(TransportBuffer, WriteArgs, Error)} method call.
 * 
 * @see Channel
 */
public class WritePriorities
{
    // WritePriorities class cannot be instantiated
    private WritePriorities()
    {
        throw new AssertionError();
    }

    /** Assigns message to the high priority flush, if not directly written to the socket. */
    public static final int HIGH = 0;
	
    /** Assigns message to the medium priority flush, if not directly written to the socket. */
    public static final int MEDIUM = 1;
	
    /** Assigns message to the low priority flush, if not directly written to the socket. */
    public static final int LOW = 2;
}
