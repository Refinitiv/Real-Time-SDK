/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;


/**
 * ETA Read Arguments used in the {@link Channel#read(ReadArgs, Error)} call.
 * 
 * @see Channel
 */
public interface ReadArgs
{
    /**
     * ETA Read Return Value used in the {@link Channel#read(ReadArgs, Error)}
     * call. A positive value if there is more data to read,
     * {@link TransportReturnCodes#READ_WOULD_BLOCK} if the read call is
     * blocked, or a failure code.
     * 
     * @return the readRetVal
     */
    public int readRetVal();

    /**
     * The number of bytes read, including any transport header overhead,
     * excluding any decompression, with this read call.
     * 
     * @return the bytesRead
     */
    public int bytesRead();

    /**
     * The number of bytes read, including any transport header overhead and
     * including any decompression, with this read call.
     * 
     * @return the uncompressedBytesRead
     */
    public int uncompressedBytesRead();

    /**
     * The sequence number associated with this packet, used in a sequenced
     * multicast connection.
     * 
     * @return the sequence number
     */
    public long seqNum();
    
    /**
     * The address of the sender of the received message.
     * 
     * @return the sender IP Address
     */
    public String senderAddress();
    
    /**
     * The port of the sender of the received message.
     * 
     * @return the sender and port
     */
    public int senderPort();
    
    /**
     * The instance ID of the sender's channel.  When combined with the senderAddress 
     * and senderPort, this can be used to identify the specific channel that sent this message.
     * 
     * @return the instanceId
     */
    public int instanceId();
    
    /**
     * Read flags used for {@link Channel#read(ReadArgs, Error)}.
     * Must be in the range of {@link ReadFlags#NO_FLAGS} - {@link ReadFlags#READ_SEQNUM}.
     * 
     * @param flags the flags to set
     * 
     * @see ReadFlags
     */
    public void flags(int flags);

    /**
     * Read flags used for {@link Channel#read(ReadArgs, Error)}.
     * 
     * @return the flags
     * 
     * @see ReadFlags
     */
    public int flags();

    /**
     * Clears ReadArgs.
     */   
    public void clear();
}
