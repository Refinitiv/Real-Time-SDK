package com.thomsonreuters.upa.perftools.upajtransportperf;


import com.thomsonreuters.upa.perftools.common.ClientChannelInfo;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.WriteArgs;
import com.thomsonreuters.upa.transport.Error;

/** Stores information about an open session on a channel.
  * Handles sending messages for a transport thread. */
public class TransportSession
{
    private final int SEQNUM_TIMESTAMP_LEN = 16; /* length of sequence number and timestamp */
    private ClientChannelInfo   _channelInfo;           /* Channel associated with this session */
    private int                 _maxMsgBufSize;         /* The buffer size to request from RSSL via rsslGetBuffer(); May vary according to fragment size and packing */
    private TransportBuffer     _writingBuffer;         /* Current buffer in use by this channel. */
    private int                 _packedBufferCount;     /* Total number of buffers currently packed in pWritingBuffer */
    private long                _sendSequenceNumber;    /* Next sequence number to send. */
    private long                _recvSequenceNumber;    /* Next sequence number that should be received. */
    private boolean             _receivedFirstSequenceNumber;   /* Indicates whether a sequence number has been received yet. */
    private long                _timeActivated;         /* Time at which this channel was fully setup. */

    private WriteArgs _writeArgs = TransportFactory.createWriteArgs();

    public TransportSession(ClientChannelInfo channelInfo)
    {
        _maxMsgBufSize = TransportThreadConfig.totalBuffersPerPack() * TransportThreadConfig.msgSize();

        /* If the buffer is to be packed, add some additional bytes for each message. */
        if (channelInfo.channel.connectionType() == ConnectionTypes.SEQUENCED_MCAST && 
                TransportThreadConfig.totalBuffersPerPack() > 1)
            _maxMsgBufSize = _maxMsgBufSize + ((TransportThreadConfig.totalBuffersPerPack() - 1) * 2);
        else if (TransportThreadConfig.totalBuffersPerPack() > 1)
            _maxMsgBufSize = _maxMsgBufSize + (TransportThreadConfig.totalBuffersPerPack() * 8);

        _channelInfo = channelInfo;
        _channelInfo.userSpec = this;
        
        _writeArgs.flags(TransportThreadConfig.writeFlags());

    }
    
    /**
     * Send a burst of messages for one tick.
     * @param handler - Transport thread sending messages.
     * @param error - Gives detailed information about error if any occurred during socket operations.
     * @return negative value in case of error, >=0 if no errors.
     */
    public int sendMsgBurst(TransportThread handler, Error error)
    {
        int msgsLeft;
        int latencyUpdateNumber;
        int ret = TransportReturnCodes.SUCCESS;

        /* Determine msgs to send out. Spread the remainder out over the first ticks */
        msgsLeft = TransportThreadConfig.msgsPerTick();
        if (TransportThreadConfig.msgsPerTickRemainder() > handler.currentTicks())
            ++msgsLeft;

        latencyUpdateNumber = (TransportThreadConfig.latencyMsgsPerSec() > 0) ?
                handler.latencyRandomArray().next() : -1; 

        for(; msgsLeft > 0; --msgsLeft)
        {
            /* Send the item. */
            ret = sendMsg(handler, msgsLeft, 
                    (msgsLeft - 1) == latencyUpdateNumber || TransportThreadConfig.latencyMsgsPerSec() == TransportThreadConfig.ALWAYS_SEND_LATENCY_MSG,
                    error);

            if (ret < TransportReturnCodes.SUCCESS)
            {
                if (ret == TransportReturnCodes.NO_BUFFERS)
                    handler.outOfBuffersCount().add(msgsLeft);

                return ret;
            }
        }

        return ret;
    }
    
    private int sendMsg(TransportThread handler, int msgsLeft, boolean sendLatency, Error error)
    {
        int ret;
        long currentTime;

        /* Add latency timestamp, if appropriate. */
        if (sendLatency)
            currentTime = System.nanoTime();
        else
            currentTime = 0;

        if ((ret = getMsgBuffer(error)) < TransportReturnCodes.SUCCESS)
            return ret;

        if (_writingBuffer.length() < TransportThreadConfig.msgSize())
        {
            System.out.printf("Error: TransportSession.sendMsg(): Buffer length %d is too small to write next message.\n", _writingBuffer.length());
            System.exit(-1);
        }
    
        /* Add sequence number */
                
        _writingBuffer.data().putLong(Long.reverseBytes(_sendSequenceNumber));

        /* Add currentTime */
        _writingBuffer.data().putLong(Long.reverseBytes(currentTime));

        /* Zero out remainder of message */

        for (int i = 0; i < TransportThreadConfig.msgSize() - SEQNUM_TIMESTAMP_LEN; i++)
        {
            _writingBuffer.data().put((byte)0);
        }

        
        if ((ret = writeMsgBuffer(handler, msgsLeft > 1, error)) >= TransportReturnCodes.SUCCESS)
        {
            /* update sequence number for next time */
            ++_sendSequenceNumber;          
        }
            
        return ret;
    }
    
    private int getMsgBuffer(Error error)
    {
        Channel chnl = _channelInfo.channel;

        if (_writingBuffer != null) 
            return TransportReturnCodes.SUCCESS;
        else
        {
            if (chnl.connectionType() == ConnectionTypes.SEQUENCED_MCAST)
                _writingBuffer = chnl.getBuffer(_maxMsgBufSize, true, error);
            else
                _writingBuffer = chnl.getBuffer(_maxMsgBufSize, (TransportThreadConfig.totalBuffersPerPack() > 1) ? true : false, error);
            if(_writingBuffer != null)
            {
                return TransportReturnCodes.SUCCESS;
            }
            else if (error.errorId() == TransportReturnCodes.FAILURE)
            {
                return TransportReturnCodes.FAILURE;
            }
            else
            {
                return TransportReturnCodes.NO_BUFFERS;
            }
        }
    }

    private int writeMsgBuffer(TransportThread handler, boolean allowPack, Error error)
    {
        int ret;
        Channel chnl = _channelInfo.channel;

        /* Make sure we stop packing at the end of a burst of msgs
         *   in case the next burst is for a different channel. 
         *   (This will also prevent any latency msgs from sitting in the pack for a tick). */
        if (_packedBufferCount == (TransportThreadConfig.totalBuffersPerPack() - 1) || !allowPack)
        {
            /* Send the completed buffer(or if there is no packing being done, send as normal) */
            _packedBufferCount = 0;

            ret = chnl.write(_writingBuffer, _writeArgs, error);
            
            /* call flush and write again */
            while (ret == TransportReturnCodes.WRITE_CALL_AGAIN)
            {
                if ((ret = chnl.flush(error)) < TransportReturnCodes.SUCCESS)
                {
                    System.out.printf("Channel.flush() failed with return code %d - <%s>\n", ret, error.text());
                    return ret;
                }
                ret = chnl.write(_writingBuffer, _writeArgs, error);
            }

            _writingBuffer = null;
            
            if (ret >= TransportReturnCodes.SUCCESS)
            {
                handler.bytesSent().add(_writeArgs.bytesWritten());
                handler.msgsSent().increment();
                return ret;
            }

            switch(ret)
            {
                case TransportReturnCodes.WRITE_FLUSH_FAILED:
                    if (chnl.state() == ChannelState.ACTIVE)
                    {
                        handler.bytesSent().add(_writeArgs.bytesWritten());
                        handler.msgsSent().increment();
                        return 1;
                    }
                    /* Otherwise treat as error, fall through to default. */
                default:
                    if (ret != TransportReturnCodes.NO_BUFFERS)
                    {
                        System.out.printf("Channel.write() failed: %d(%s)\n", ret, error.text());
                    }
                    return ret;
            }
        }
        else
        {
            /* Pack the buffer and continue using it. */
            ++_packedBufferCount;
            ret = chnl.packBuffer(_writingBuffer, error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                System.out.printf("Channel.packBuffer failed: %d <%s>", error.errorId(), error.text());
                return TransportReturnCodes.FAILURE;
            }
            handler.msgsSent().increment();
            return TransportReturnCodes.SUCCESS;
        }
    }
    
    /**
     * 
     * @return Channel associated with this session.
     */
    public ClientChannelInfo channelInfo()
    {
        return _channelInfo;
    }

    /**
     * 
     * @return recvSequenceNumber Next sequence number that should be received. 
     */
    public long recvSequenceNumber()
    {
        return _recvSequenceNumber;
    }
    
    /**
     * 
     * @param recvSequenceNumber Next sequence number that should be received.
     */
    public void recvSequenceNumber(long recvSequenceNumber)
    {
        _recvSequenceNumber = recvSequenceNumber;
    }
    
    /**
     * 
     * @return receivedFirstSequenceNumber which indicates whether a sequence number has been received yet. 
     */
    public boolean receivedFirstSequenceNumber()
    {
        return _receivedFirstSequenceNumber;
    }
    
    /**
     * 
     * @param receivedFirstSequenceNumber which indicates whether a sequence number has been received yet.
     */
    public void receivedFirstSequenceNumber(boolean receivedFirstSequenceNumber)
    {
        _receivedFirstSequenceNumber = receivedFirstSequenceNumber;
    }

    /**
     * 
     * @return Time at which this channel was fully setup. 
     */
    public long timeActivated()
    {
        return _timeActivated;
    }
    
    /**
     * 
     * @param timeActivated Time at which this channel was fully setup. 
     */
    public void timeActivated(long timeActivated)
    {
        _timeActivated = timeActivated;
    }
}
