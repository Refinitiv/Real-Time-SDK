/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.queue;

/**
 * Special timeout values used when sending queue data messages.
 */
public class QueueDataTimeoutCode
{
    /**
     * The message will expire at a value chosen by the provider.
     */
    public static final long PROVIDER_DEFAULT = -2;

    /**
     * The messages has no timeout and does not expire.
     */
    public static final long INFINITE = -1;
    
    /**
     * The message should expire immediately if the
     * recipient queue is not online.
     */
    public static final long IMMEDIATE = 0;

    public static String toString(long timeoutCode)
    {
        if (timeoutCode == PROVIDER_DEFAULT)
            return "PROVIDER_DEFAULT";
		else if (timeoutCode == INFINITE)
            return "INFINITE";
        else if (timeoutCode == IMMEDIATE)
            return "IMMEDIATE";
        else
            return "Unknown";
    }
}
