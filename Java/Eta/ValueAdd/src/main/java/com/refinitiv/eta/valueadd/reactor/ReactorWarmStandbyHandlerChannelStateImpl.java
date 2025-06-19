/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Enumerated types indicating warm standby handler channel state. See {@link ReactorWarmStandbyHandler}.
 */
class ReactorWarmStandbyHandlerChannelStateImpl {
    // ReactorWarmStandbyHandlerChannelState class cannot be instantiated
    private ReactorWarmStandbyHandlerChannelStateImpl()
    {
        throw new AssertionError();
    }
    
    public static final int INITIALIZING =  0;
    public static final int DOWN =  1;
    public static final int DOWN_RECONNECTING = 2;
    public static final int UP = 3;
    public static final int READY = 4;
}
