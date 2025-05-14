/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.io.IOException;
import java.nio.ByteBuffer;

interface ProtocolFunctions
{	
	/* Read additional transport header if any */
	int readPrependTransportHdr(int bytesRead, ReadArgsImpl readArgs, Error error);
	
	/* Write additional transport header if any */
	int prependTransportHdr(TransportBufferImpl buffer, int ripcFlags);

	int prependInitChnlHdr(ByteBuffer sourceData, ByteBuffer destinationData);

	int initChnlReadFromChannel(ByteBuffer sourceData, Error error) throws IOException;

	int initChnlReadFromChannelProvider(ByteBuffer dst, Error error) throws IOException;
	
	/* This is used to get additional header length for a transport type. */
	int additionalHdrLength();
	
	/* This method includes header and message length */
	int messageLength();
	
	/* Clears the message length for each protocol types.*/
	void unsetMessageLength();
	
	/* This function doesn't include the packed header length if any */
	int estimateHeaderLength();
	
	/* Returns the entire header length of a message */
	int entireHeaderLength();

	/* Writes additional message prefix for a transport type. */
	int writeAdditionalMessagePrefix(TransportBufferImpl buffer);
	
	 /* Update the packed header with the length or JSON array delimiter of the message being packed.
     * 
     * reserveNextPackedHdr is true, if this method will attempt to reserve space (i.e. the packed header) for another packed message.
     * 
     * Returns the amount of user available bytes remaining for packing.
     */
	int packBuffer(TransportBufferImpl packedBuffer, boolean reserveNextPackedHdr, Channel channel, Error error);
	
	/* Checks whether the compression is needed for sending the message. */
	default boolean compressedData(RsslSocketChannel rsslSocketChannel, int messageLength, WriteArgs writeArgs)
	{
		if (rsslSocketChannel._sessionOutCompression > 0 && (writeArgs.flags() & WriteFlags.DO_NOT_COMPRESS) == 0)
        {
            // only compress if within low and high thresholds
            if (messageLength >= rsslSocketChannel._sessionCompLowThreshold)
            {
                // set _compressPriority if necessary
                if (rsslSocketChannel._compressPriority == 99)
                {
               	    rsslSocketChannel._compressPriority = writeArgs.priority();
                }
                
                // only compress with initial message priority
                if (writeArgs.priority() == rsslSocketChannel._compressPriority)
                {
                    return true;
                }
            }
        }
		
		return false;
	}
	
	/* Compress the buffer and write according to the connection type */
	int writeCompressed(TransportBufferImpl buffer, WriteArgs writeArgs, Error error);
	
	/* Gets a big buffer for handling message fragmentation */
	TransportBufferImpl getBigBuffer(int size);
	
	/* Populates fragmented messages without compression */
	int populateFragment(BigBuffer bigBuffer, boolean firstFragment, TransportBufferImpl writeBuffer, int flags, WriteArgs writeArgs);
	
	/* Populates fragmented messages with compression */
	int writeFragmentCompressed(BigBuffer bigBuffer, TransportBufferImpl fragment, WriteArgs writeArgs, boolean firstFragment, Error error);
	
	/* Write an additional character for ending the big buffer for the JSON protocol. Returns the length of the additional suffix */
	int writeFragmentSuffix(BigBuffer bigBuffer);
	
	/* Checks whether to compare the specified message size with the low compression threshold */
	boolean checkCompressionFragmentedMsg(int messageSize);
	
	/* Checks whether the current message is a ping message*/
	boolean isPingMessage();

	int ping(Error error) throws IOException;

	int pong(Error error) throws IOException;

	int closeChannel(Error error);

	boolean isRWFProtocol();
	
	boolean writeAsFragmentedMessage(TransportBufferImpl buffer);
	
	/* Gets the actual payload size for big buffer with compression over JSON2 protocol. 
	 * Returns non-zero if the payload size is changed.
	 * */
	int totalPayloadSize();
	
	int remaingBytesAfterPausing(BigBuffer bigBuffer);
}
