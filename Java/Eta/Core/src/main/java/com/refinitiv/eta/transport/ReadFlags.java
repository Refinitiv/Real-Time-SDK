/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;


/**
 * ETA Read Flags passed into the {@link Channel#read(ReadArgs, Error)} method call.
 * 
 * @see Channel
 */
public class ReadFlags
{
    // ReadFlags class cannot be instantiated
    private ReadFlags()
    {
        throw new AssertionError();
    }

    /**
     * No modification will be performed to this {@link Channel#read(ReadArgs, Error)} operation.
     */
    public static final int NO_FLAGS = 0x00;
   
    /**
     * set when valid node id is returned
     */

    public static final int READ_NODE_ID = 0x02;    
    
    /**
     * set when sequence number is returned
     */

    public static final int READ_SEQNUM = 0x04;
    
    /**
     * set when the message has an instance ID set
     */

    public static final int READ_INSTANCE_ID = 0x20;
    
    /**
     *  indicates that this message is a retransmission of previous content
     */
    
    public static final int READ_RETRANSMIT = 0x40;    
    
    /*
     * This is the max combined value of the bits mask that is allowed.
     */
    static final int MAX_VALUE = 0x40;
}

