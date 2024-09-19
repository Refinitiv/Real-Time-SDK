/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.transportperf;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.perftools.common.ClientChannelInfo;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportReturnCodes;

/** Process messages as a reader/writer. */
public class ProcessMsgReadWrite implements ProcessMsg
{

    private static final int PING_MSG_SIZE = 17;

    @Override
    public int processMsg(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, TransportBuffer msgBuffer, Error error)
    {
        int minMsgSize = Long.BYTES * 2;
        boolean jsonProtocol = channelInfo.channel.protocolType() == Codec.JSON_PROTOCOL_TYPE;
        SessionHandler handler = (SessionHandler)channelHandler.userSpec();
        TransportSession session = (TransportSession)channelInfo.userSpec;
        long timeTracker;

        handler.transportThread().msgsReceived().increment();
        handler.transportThread().bytesReceived().add(msgBuffer.length());

        if (jsonProtocol) {
            minMsgSize += Integer.BYTES;
            if (msgBuffer.length() == PING_MSG_SIZE) {
                return TransportReturnCodes.READ_PING;
            }
        }

        if (msgBuffer.length() < minMsgSize)
        {
        	error.errorId(TransportReturnCodes.FAILURE);
        	error.text("Error: Message was too small to be valid(length " + msgBuffer.length() + ").\n");
            return TransportReturnCodes.FAILURE;
        }
        
        ByteBuffer byteBuffer = msgBuffer.data();
        int msgLen = 0;
        int startPosition = byteBuffer.position();
        // Unpack JSON buffer.
        do {

            if (channelInfo.channel.protocolType() == Codec.JSON_PROTOCOL_TYPE) {
                char symbol = (char) byteBuffer.get();
                if (symbol == ']') {
                    break;
                } else if (symbol != '[' && symbol != ',') {
                    byteBuffer.position(byteBuffer.position() - 1);
                }
                msgLen = Integer.reverseBytes(byteBuffer.getInt());
            }

            if (session.receivedFirstSequenceNumber()) {
                long recvSequenceNumber;

                recvSequenceNumber = Long.reverseBytes(msgBuffer.data().getLong());

                if (session.recvSequenceNumber() != recvSequenceNumber) {
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.text("Error: Received out-of-order sequence number(" + recvSequenceNumber + " instead of " + session.recvSequenceNumber() + ").");
                    return TransportReturnCodes.FAILURE;
                }

                session.recvSequenceNumber(session.recvSequenceNumber() + 1);

                timeTracker = Long.reverseBytes(msgBuffer.data().getLong());

                if (timeTracker > 0) {
                    handler.transportThread().timeRecordSubmit(handler.latencyRecords(), timeTracker, System.nanoTime(), 1000);
                }
            } else {
                session.recvSequenceNumber(Long.reverseBytes(byteBuffer.getLong()));
                session.receivedFirstSequenceNumber(true);
                session.recvSequenceNumber(session.recvSequenceNumber() + 1);
            }

            if (channelInfo.channel.protocolType() == Codec.JSON_PROTOCOL_TYPE) {
                byteBuffer.position(startPosition + 1 + msgLen);
            }
        } while (channelInfo.channel.protocolType() == Codec.JSON_PROTOCOL_TYPE && byteBuffer.position() <= byteBuffer.limit());
        return TransportReturnCodes.SUCCESS;
    }
}
