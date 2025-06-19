/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.queue;

/**
 * The types of RDM queue messages. rdmMsgType member in {@link QueueMsg} may
 * be set to one of these to indicate the specific RDMQueueMsg class.
 * 
 * @see QueueClose
 * @see QueueRefresh
 * @see QueueRequest
 * @see QueueStatus
 * @see QueueAck
 * @see QueueData
 * @see QueueDataExpired
 */
public enum QueueMsgType
{
    /**
     * (0) Unknown
     */
    UNKNOWN(0),

    /** (1) Queue Request */
    REQUEST(1),
    
    /** (2) Queue Close */
    CLOSE(2), 
    
    /** (3) Queue Refresh */
    REFRESH(3), 
    
    /** (4) Queue Status */
    STATUS(4), 

    /** (5) Queue Ack */
    ACK(5),

    /** (6) Queue Data */
    DATA(6),
    
    /** (7) Queue Data Expired */
    DATAEXPIRED(7);    

    
    private QueueMsgType(int value)
    {
        this.value = value;
    }

    @SuppressWarnings("unused")
    private int value;
}
