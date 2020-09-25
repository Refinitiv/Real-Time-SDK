package com.refinitiv.eta.perftools.transportperf;

import java.nio.ByteBuffer;

import com.refinitiv.eta.perftools.common.ClientChannelInfo;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportReturnCodes;

/** Process messages as a reader/writer. */
public class ProcessMsgReadWrite implements ProcessMsg
{
    @Override
    public int processMsg(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, TransportBuffer msgBuffer, Error error)
    {
        SessionHandler handler = (SessionHandler)channelHandler.userSpec();
        TransportSession session = (TransportSession)channelInfo.userSpec;
        long timeTracker;

        handler.transportThread().msgsReceived().increment();
        handler.transportThread().bytesReceived().add(msgBuffer.length());

        if (msgBuffer.length() < 16)
        {
        	error.errorId(TransportReturnCodes.FAILURE);
        	error.text("Error: Message was too small to be valid(length " + msgBuffer.length() + ").\n");
            return TransportReturnCodes.FAILURE;
        }
        
        ByteBuffer byteBuffer = msgBuffer.data();

        if (session.receivedFirstSequenceNumber())
        {       
            long recvSequenceNumber;
            
            recvSequenceNumber = Long.reverseBytes(msgBuffer.data().getLong());
            
            if (session.recvSequenceNumber() != recvSequenceNumber)
            {
            	error.errorId(TransportReturnCodes.FAILURE);
                error.text("Error: Received out-of-order sequence number(" + recvSequenceNumber + " instead of " + session.recvSequenceNumber() + ").");
                return TransportReturnCodes.FAILURE;
            }

            session.recvSequenceNumber(session.recvSequenceNumber() + 1);

            timeTracker = Long.reverseBytes(msgBuffer.data().getLong());            
            
            if (timeTracker > 0)
            {
                handler.transportThread().timeRecordSubmit(handler.latencyRecords(), timeTracker, System.nanoTime(), 1000);
            }
            return TransportReturnCodes.SUCCESS;
        }
        else
        {           
            session.recvSequenceNumber(Long.reverseBytes(byteBuffer.getLong()));
            session.receivedFirstSequenceNumber(true);
            session.recvSequenceNumber(session.recvSequenceNumber() + 1);
            return TransportReturnCodes.SUCCESS;
        }
    }
}
