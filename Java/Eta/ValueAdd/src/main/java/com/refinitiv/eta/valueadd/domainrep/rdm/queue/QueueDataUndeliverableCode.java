/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.queue;

/** Codes for undeliverable queue data messages.  */
public class QueueDataUndeliverableCode
{
    /** Unspecified */
    public static final int UNSPECIFIED = 0;

    /** The time limit on this message expired */
    public static final int EXPIRED = 1;

    /** Sender is not permitted to send to this message's destination */
    public static final int NO_PERMISSION = 2;

    /** Destination of this message does not exist */
    public static final int INVALID_TARGET = 3;

    /** Destination's message queue is full */
    public static final int QUEUE_FULL = 4;

    /** Destination's queue has been disabled */
    public static final int QUEUE_DISABLED = 5;
    
    /** Message was too large */
    public static final int MAX_MSG_SIZE = 6;
    
    /** The sender of this message is now invalid */
    public static final int INVALID_SENDER = 7;
    
    /** The target queue was deleted after sending the message, but before it was delivered */
    public static final int TARGET_DELETED = 8;

    public static String toString(int expirationCode)
    {
        switch(expirationCode)
        {
            case UNSPECIFIED:
                return "UNSPECIFIED";
            case EXPIRED:
                return "EXPIRED";
            case NO_PERMISSION:
                return "NO_PERMISSION";
            case INVALID_TARGET:
                return "INVALID_TARGET";
            case QUEUE_FULL:
                return "QUEUE_FULL";
            case QUEUE_DISABLED:
                return "QUEUE_DISABLED";
            case MAX_MSG_SIZE:
            	return "MAX_MSG_SIZE";
            case INVALID_SENDER:
            	return "INVALID_SENDER";
            case TARGET_DELETED:
            	return "TARGET_DELETED";
            default:
                return "Unknown";
        }
    }
}
