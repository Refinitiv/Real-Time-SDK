///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.test.network.replay;

/**
 * Creates {@link NetworkReplay} instances
 */
public final class NetworkReplayFactory
{
    /**
     * This class is not instantiated
     */
    private NetworkReplayFactory()
    {
        throw new AssertionError();
    }
    
    /**
     * Creates a new implementation of a {@link NetworkReplay}
     * 
     * @return a new implementation of a {@link NetworkReplay}
     */
    public static NetworkReplay create()
    {
        return new NetworkReplayImpl();
    }
}
