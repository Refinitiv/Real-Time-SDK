package com.refinitiv.eta.valueadd.reactor;

import java.nio.ByteBuffer;

import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.common.VaDoubleLinkList.Link;
import com.refinitiv.eta.valueadd.reactor.SlicedBufferPool.DuplicateBuffer;
import com.refinitiv.eta.valueadd.reactor.SlicedBufferPool.SliceableBuffer;


/* Buffer used by TunnelStreams. This is the buffer returned by
 * TunnelStream.getBuffer() and is used both by the application
 * and internally to send data over the tunnel stream. */ 
class TunnelStreamBuffer implements TransportBuffer
{
	boolean _isBigBuffer; // flag to indicate big buffer
	
    /* The actual data represented as a ByteBuffer. */
    protected ByteBuffer _data;

 	protected int _startPos, _encodedPosition;
 	private int _innerCapacity;
	private int _seqNum;
	int _containerType;
	SliceableBuffer _parentBuffer; // used by SlicedBufferPool
	DuplicateBuffer _duplicateBuffer; // used by SlicedBufferPool
	boolean _isUserBuffer; // used by SlicedBufferPool
	boolean _isForLocalAck; // Used when generating local acks; indicates this message was acknowledged.
	private int _tunnelStreamHeaderLen;

    /* Persistence associated with this buffer, if any. 
     * Mainly needed in case a buffer expires before transmission (so we can make sure the 
     * persistence is freed). */
    TunnelStreamPersistenceBuffer _persistenceBuffer;

    /* Substream associated with persistent buffer, if any. */
    TunnelSubstream _tunnelSubstream;
    
    /* Control's buffer's behavior with regard to dataStartPos() and length().
     * It needs to be different depending on what portion is in use for the
     * encode/decode iterators to work correctly. */
    private enum Mode
    {
        /* Buffer's length/dataStartPos reflect only the encoded content inside the buffer
         * (i.e. not including the TunnelStream message header) as a TransportBuffer for writing. */
        INNER_WRITE_BUFFER,

        /* Buffer's length/dataStartPos reflect the full buffer (i.e. including the TunnelStream
         * message header) */
        FULL_WRITE_BUFFER,

        /* Buffer's length/dataStartPos reflect the full buffer (i.e. including the TunnelStream 
         * message header). Mainly for internally updating the TunnelStream message header. */
        FULL_READ_BUFFER,

        /* Buffer's length/dataStartPos reflect only the encoded content inside the buffer
         * (i.e. not including the TunnelStream message header) as a TransportBuffer for reading. */
        INNER_READ_BUFFER

    }

    Mode _mode;

	static final int MAX_SUBSTREAM_HEADER_LENGTH = 512;

	private long _timeQueued;
    private long _timeout;
	private boolean _timeoutIsCode;
	private boolean _isApplicationBuffer;
	private boolean _isRetransmit;
	private boolean _isWaitingAck;
	private boolean _isQueueData; /* Remembers whether the buffer contains QueueData, in case we need to wait for a QueueAck. */
    private boolean _isQueueClose; /* Remember whether the buffer contains a QueueClose. At this point we should close the persist file if open. */
    private boolean _isTransmitted;
	
	/* Buffer link for transmission lists. */
	private TunnelStreamBuffer _retransNext, _retransPrev;
	static class BufferRetransmitLink implements Link<TunnelStreamBuffer>
	{
		public TunnelStreamBuffer getPrev(TunnelStreamBuffer thisPrev) { return thisPrev._retransPrev; }
		public void setPrev(TunnelStreamBuffer thisPrev, TunnelStreamBuffer thatPrev) { thisPrev._retransPrev = thatPrev; }
		public TunnelStreamBuffer getNext(TunnelStreamBuffer thisNext) { return thisNext._retransNext; }
		public void setNext(TunnelStreamBuffer thisNext, TunnelStreamBuffer thatNext) { thisNext._retransNext = thatNext; }
	}
	static final BufferRetransmitLink RETRANS_LINK = new BufferRetransmitLink();

	/* Buffer link for timeout lists. */
	private TunnelStreamBuffer _timeoutNext, _timeoutPrev; static class TimeoutLink implements Link<TunnelStreamBuffer>
	{
		public TunnelStreamBuffer getPrev(TunnelStreamBuffer thisPrev) { return thisPrev._timeoutPrev; }
		public void setPrev(TunnelStreamBuffer thisPrev, TunnelStreamBuffer thatPrev) { thisPrev._timeoutPrev = thatPrev; }
		public TunnelStreamBuffer getNext(TunnelStreamBuffer thisNext) { return thisNext._timeoutNext; }
		public void setNext(TunnelStreamBuffer thisNext, TunnelStreamBuffer thatNext) { thisNext._timeoutNext = thatNext; }
	}
	static final TimeoutLink TIMEOUT_LINK = new TimeoutLink();


    /** Returns the persistence buffer associated with this buffer, if any. */
    TunnelStreamPersistenceBuffer persistenceBuffer()
    {
        return _persistenceBuffer;
    }

    /** Returns the substream associated with this buffer. */
    TunnelSubstream tunnelSubstream()
    {
        return _tunnelSubstream;
    }

    /** Associate this buffer with persistence. */
    void persistenceBuffer(TunnelSubstream tunnelSubstream, TunnelStreamPersistenceBuffer persistenceBuffer)
    {
        _tunnelSubstream = tunnelSubstream;
        _persistenceBuffer = persistenceBuffer;
    }


	int seqNum()
	{
		return _seqNum;
	}

	void seqNum(int seqNum)
	{
		_seqNum = seqNum;
	}

	public ByteBuffer data()
	{
		return _data;
	}
	
	void data(ByteBuffer buffer)
	{
		_data = buffer;
	}
	
	void data(ByteBuffer buffer, int position, int length)
	{
		_data = buffer;
		_startPos = position;
		_data.position(position);
		_data.limit(position + length);
	}

	/** Set the buffer's position/limit/length to represent 
	 * the full outbound buffer (i.e. including the TunnelStream header). */
    void setToFullWritebuffer()
    {
        _data.limit(_startPos + _tunnelStreamHeaderLen + _innerCapacity);
        _data.position(_encodedPosition);
        _mode = Mode.FULL_WRITE_BUFFER;
    }

	/** Set the buffer's position/limit/length to represent 
	 * only the content of the outbound buffer (i.e. NOT including the TunnelStream header).
	 * This is how the application should see buffers it gets for writing. */
    void setToInnerWriteBuffer()
    {
        _data.limit(_startPos + _tunnelStreamHeaderLen + _innerCapacity);
        _data.position(_encodedPosition);
        _mode = Mode.INNER_WRITE_BUFFER;
    }
    
    /** For outbound buffers. Returns the length of the inner buffer. */
    int innerWriteBufferLength()
    {
        return (_encodedPosition - (_startPos + _tunnelStreamHeaderLen));
    }

	/** Set the buffer's position/limit/length to represent 
	 * the full inbound buffer (i.e. including the TunnelStream header). */
    void setAsFullReadBuffer()
    {
        _mode = Mode.FULL_READ_BUFFER;
        _data.limit(_encodedPosition);
        _data.position(_startPos);
    }

	/** Set the buffer's position/limit/length to represent 
	 * only the content of the inbound buffer (i.e. NOT including the TunnelStream header). 
	 * This is how the application should see buffers for reading
	 * (Mainly used for internally-generated QueueAcks or QueueDataExpireds) */
    void setAsInnerReadBuffer()
    {
        _mode = Mode.INNER_READ_BUFFER;
        _data.limit(_encodedPosition);
        _data.position(_startPos + _tunnelStreamHeaderLen);
    }

    /** Sets the current position as the end of encoded content. */
    void setCurrentPositionAsEndOfEncoding()
    {
        _encodedPosition = _data.position();
    }

    @Override
    public int capacity()
    {
        switch(_mode)
        {
            case INNER_WRITE_BUFFER:
                return _innerCapacity;
            case INNER_READ_BUFFER:
                return _encodedPosition - (_startPos + _tunnelStreamHeaderLen);
            case FULL_READ_BUFFER:
                return _encodedPosition - _startPos;
            default: /* FULL_WRITE_BUFFER */
                return _innerCapacity + _tunnelStreamHeaderLen;

        }
    }

	@Override
    public int copy(ByteBuffer destBuffer)
	{
		int tmpPos, tmpLimit;
		int tmpDestPos;
		int length = length();
		int startPos = _startPos + _tunnelStreamHeaderLen;

		if (destBuffer == null)
			return ReactorReturnCodes.INVALID_USAGE;

		if (destBuffer.limit() - destBuffer.position() < length)
			return ReactorReturnCodes.INVALID_USAGE;

		tmpPos = _data.position();
		tmpLimit = _data.limit();
		
		
		_data.position(startPos);
		_data.limit(startPos + length);
		tmpDestPos = destBuffer.position();
		destBuffer.put(_data);
		destBuffer.position(tmpDestPos);
		
		_data.limit(tmpLimit);
		_data.position(tmpPos);
        

		return ReactorReturnCodes.SUCCESS;
	}

	void containerType(int containerType)
	{
		_containerType = containerType;
	}

    @Override
	public int dataStartPosition()
	{
        switch(_mode)
        {
            case INNER_WRITE_BUFFER:
            case INNER_READ_BUFFER:
                return _startPos + _tunnelStreamHeaderLen;
            default: /* FULL_WRITE_BUFFER, FULL_READ_BUFFER */
                return _startPos;
        }
	}

    int tunnelBufferStartPosition()
    {
        return _startPos;
    }

    boolean isForLocalAck() { return _isForLocalAck; }

    void isForLocalAck(boolean isForLocalAck) { _isForLocalAck = isForLocalAck; }

    int tunnelStreamHeaderLen()
    {
        return _tunnelStreamHeaderLen;
    }   

    void tunnelStreamHeaderLen(int length)
    {
        _tunnelStreamHeaderLen = length;
    }   

    @Override
	public int length()
	{		
        if (data() == null)
              return 0;

        switch(_mode)
        {
            case INNER_WRITE_BUFFER:
                if (data().position() > _startPos + _tunnelStreamHeaderLen)
                    /* Application has written content. Return the length of that content. */
                    return data().position() - (_startPos + _tunnelStreamHeaderLen);
                else
                    /* At the start of application's encoding. Return capacity. */
                    return _innerCapacity;

            case INNER_READ_BUFFER:
                /* Return length of encoded content. */
                return data().limit() - (_startPos + _tunnelStreamHeaderLen);

            case FULL_READ_BUFFER:
                /* Return full length (encoded content and TunnelStream header). */
                return data().limit() - _startPos;

            default: /* FULL_WRITE_BUFFER */
                if (data().position() > _startPos)
                    /* TunnelStream Header (and possibly content) is encoded. */
                    return data().position() - _startPos;
                else
                    /* Nothing encoded; return full capacity. */
                    return _innerCapacity + _tunnelStreamHeaderLen;
        }
	}

	void clear(int length)
	{        
        _innerCapacity = length;
        _data = null;
        
        _tunnelStreamHeaderLen = 0;
        _encodedPosition = 0;
        _mode = Mode.FULL_WRITE_BUFFER;
		
		_timeout = 0;
		_timeoutIsCode = false;
        _isTransmitted = false;
		_isApplicationBuffer = true;
		_isRetransmit = false;
		_isWaitingAck = false;
		_isQueueData = false;
		_isForLocalAck = false;
        _persistenceBuffer = null;
        _tunnelSubstream = null;
	}

	long timeQueuedNsec() { return _timeQueued; }
	void timeQueuedNsec(long timeQueued) { _timeQueued = timeQueued; }
	long timeoutNsec() { return _timeout; }
	void timeoutNsec(long timeout) { _timeout = timeout; }
	boolean timeoutIsCode() { return _timeoutIsCode; }
	void timeoutIsCode(boolean timeoutIsCode) { _timeoutIsCode = timeoutIsCode; }

	boolean isTransmitted() { return _isTransmitted; }
	void isTransmitted(boolean isTransmitted) { _isTransmitted = isTransmitted; }
	boolean isRetransmit() { return _isRetransmit; }
	void isRetransmit(boolean isRetransmit) { _isRetransmit = isRetransmit; }

	boolean isWaitingAck() { return _isWaitingAck; }
	void isWaitingAck(boolean isWaitingAck) { _isWaitingAck = isWaitingAck; }

	void isApplicationBuffer(boolean isApplicationBuffer) { _isApplicationBuffer = isApplicationBuffer; }
	boolean isApplicationBuffer() { return _isApplicationBuffer; }

    void isQueueData(boolean isQueueData) { _isQueueData = isQueueData; }
    boolean isQueueData() { return _isQueueData; }

    void isQueueClose(boolean isQueueClose) { _isQueueClose = isQueueClose; }
    boolean isQueueClose() { return _isQueueClose; }

    /* Copies the full buffer (including TunnelStream header). */
    void copyFullBuffer(ByteBuffer destBuffer)
    {
		int tmpPos, tmpLimit;

		tmpPos = data().position();
		tmpLimit = data().limit();
		
		data().position(_startPos);
        data().limit(_encodedPosition);
		destBuffer.put(data());
		
		data().limit(tmpLimit);
		data().position(tmpPos);
	}
    
    boolean isBigBuffer()
    {
    	return _isBigBuffer;
    }
}
