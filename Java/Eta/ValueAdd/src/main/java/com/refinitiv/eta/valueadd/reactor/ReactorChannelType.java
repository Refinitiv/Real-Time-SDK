/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Reactor Channel types.
 */
public class ReactorChannelType {
    // ReactorChannelType class cannot be instantiated
    private ReactorChannelType()
    {
        throw new AssertionError();
    }

    public static final int NORMAL = 0;			// Represents a normal Reactor channel which handles an individual channel.
    public static final int WARM_STANDBY = 1;	// Represents a Reactor warm standby channel which handles a list of channels.	
}
