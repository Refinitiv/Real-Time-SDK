/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * Options used for configuring Sequenced Multicast specific transport options (ConnectionTypes.SEQUENCED_MULTICAST).
 * 
 * @see ConnectOptions
 */

public interface SeqMCastOpts
{
    /**
     * Only used with connectionType of SEQUENCED_MULTICAST.
     * 
     * @param size The maximum length of the message
     */
    public void maxMsgSize(int size);

    /**
     * Returns the maximum message size.
     * 
     * @return maxMsgSize
     */
    public int maxMsgSize();
    
    /**
     * When combined with the sender's IP address and port, this is used to uniquely identify a sequenced multicast channel.  
     * This is a 16-bit unsigned value, with a valid range of 0-65535.
     *
     * @param id the id
     */
    public void instanceId(int id);
    
    /**
     * Returns the instance ID;.
     *
     * @return instanceId
     */
    public int instanceId();
    
}
