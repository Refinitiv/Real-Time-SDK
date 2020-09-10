package com.rtsdk.eta.valueadd.reactor;

import java.nio.ByteBuffer;

import com.rtsdk.eta.valueadd.common.VaDoubleLinkList;
import com.rtsdk.eta.valueadd.common.VaDoubleLinkList.Link;

class TunnelStreamBigBuffer extends TunnelStreamBuffer
{    
	/* Link for TunnelStreamBigBufferPool */
    private TunnelStreamBigBuffer _reactorChannelNext, _reactorChannelPrev;
    static class TunnelStreamBigBufferLink implements Link<TunnelStreamBigBuffer>
    {
        public TunnelStreamBigBuffer getPrev(TunnelStreamBigBuffer thisPrev) { return thisPrev._reactorChannelPrev; }
        public void setPrev(TunnelStreamBigBuffer thisPrev, TunnelStreamBigBuffer thatPrev) { thisPrev._reactorChannelPrev = thatPrev; }
        public TunnelStreamBigBuffer getNext(TunnelStreamBigBuffer thisNext) { return thisNext._reactorChannelNext; }
        public void setNext(TunnelStreamBigBuffer thisNext, TunnelStreamBigBuffer thatNext) { thisNext._reactorChannelNext = thatNext; }
    }
    static final TunnelStreamBigBufferLink BIG_BUFFER_LINK = new TunnelStreamBigBufferLink();
    
	VaDoubleLinkList<TunnelStreamBigBuffer> _pool;
	int _size;
	
	// information for saving progress of partially written buffer
	boolean _inProgress; // flag that indicates write is in progress
	int _totalMsgLength; // total message length
	int _bytesRemainingToSend; // length remaining to send
	int _bytesAlreadyCopied; // length of bytes on the buffer
	int _lastFragmentId; // last fragment id used while sending 
	int _messageId; // message id
	int _containerType; // container type
	
    TunnelStreamBigBuffer(VaDoubleLinkList<TunnelStreamBigBuffer> pool, int bufferSize, int userSize)
    {
    	_isBigBuffer = true;
    	_pool = pool;
    	_size = userSize;
		_data = ByteBuffer.allocate(bufferSize);
		_data.limit(userSize);
	}
    
    VaDoubleLinkList<TunnelStreamBigBuffer> pool()
    {
    	return _pool;
    }
    
    @Override
	public int length()
	{
    	int length = _encodedPosition > 0 ? _encodedPosition : _size;
    	int position = _data.position();
    	
    	if (position > 0)
    	{
    		length = position; 
    	}

    	return length;
	}
    
    @Override
	public int capacity()
	{
    	return _size;
	}
    
    @Override
	public int dataStartPosition()
	{
    	return _startPos;
	}
    
    /* Clears the object for re-use. */
    void clear(int userSize)
    {
    	_size = userSize;
    	_data.clear();
    	_data.limit(userSize);
    	_inProgress = false;
    	_totalMsgLength = 0;
    	_bytesRemainingToSend = 0;
    	_bytesAlreadyCopied = 0;
    	_lastFragmentId = 0;
    	_messageId = 0;
    	_containerType = 0;
    }
    
    /* Saves the write progress for fragmentation. */
    void saveWriteProgress(int totalMsgLength, int bytesRemaining, int lastFragmentId, int messageId, int containerType)
    {
    	_inProgress = true;
    	_totalMsgLength = totalMsgLength;
    	_bytesRemainingToSend = bytesRemaining;
    	_lastFragmentId = lastFragmentId;
    	_messageId = messageId;
    	_containerType = containerType;
    }
    
    /* Returns whether or not fragmentation is already in progress. */
    boolean fragmentationInProgress()
    {
    	return _inProgress;
    }

    /* Returns the bytes remaining to send. */
	int bytesRemainingToSend()
	{
		return _bytesRemainingToSend;
    }
	
    /* Returns the bytes currently on the buffer. */
	int bytesAlreadyCopied()
	{
		return _bytesAlreadyCopied;
    }
	
    /* Returns the last fragment id. */
	int lastFragmentId()
	{
		return _lastFragmentId;
	}
	
    /* Returns the message id. */
	int messageId()
	{
		return _messageId;
	}

    /* Returns the container type. */
	int containerType()
	{
		return _containerType;
    }
}
