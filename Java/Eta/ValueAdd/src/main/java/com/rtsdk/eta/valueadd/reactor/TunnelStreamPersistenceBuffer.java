package com.rtsdk.eta.valueadd.reactor;

import com.rtsdk.eta.valueadd.common.VaDoubleLinkList.Link;

class TunnelStreamPersistenceBuffer
{
    /* File position represented by this buffer. */
	private int _filePosition;

    /* Length of the substream data stored in this buffer, if any. */
    private int _length;

    /* Sequence number of the message stored in this buffer, if any. */
    private int _seqNum;
    
    /* Remembers whether the buffer has been transmitted before (so we don't attempt to time it out). */
    private boolean _isTransmitted;

    /* Length of tunnel stream header (header is stored with messages in older file format). */
    private int _tunnelStreamHeaderLen;

    /* Link for substream queues. */
	private TunnelStreamPersistenceBuffer _next, _prev;
	static class SubstreamLink implements Link<TunnelStreamPersistenceBuffer>
	{
		public TunnelStreamPersistenceBuffer getPrev(TunnelStreamPersistenceBuffer thisPrev) { return thisPrev._prev; }
		public void setPrev(TunnelStreamPersistenceBuffer thisPrev, TunnelStreamPersistenceBuffer thatPrev) { thisPrev._prev = thatPrev; }
		public TunnelStreamPersistenceBuffer getNext(TunnelStreamPersistenceBuffer thisNext) { return thisNext._next; }
		public void setNext(TunnelStreamPersistenceBuffer thisNext, TunnelStreamPersistenceBuffer thatNext) { thisNext._next = thatNext; }
	}
	static final SubstreamLink SUBSTREAM_LINK = new SubstreamLink();

    /** Returns the position of the persistence buffer represented by this object. */
    int filePosition()
    {
        return _filePosition;
    }
    
    /** Sets the file position of the buffer. */
    void filePosition(int filePosition)
    {
        _filePosition = filePosition;
    }

    /** Returns the length of the substream data in this buffer. */
    int length()
    {
        return _length;
    }
    
    /** Sets the length of the substream data in this buffer. */
    void length(int length)
    {
        _length = length;
    }

    /** Mark message as transmitted. */
    void isTransmitted(boolean isTransmitted)
    {
        _isTransmitted = isTransmitted;
    }
    
    /** Returns whether this buffer was transmitted. */
    boolean isTransmitted()
    {
        return _isTransmitted;
    }
    
    /** Return sequence number of persisted buffer. */
    int seqNum()
    {
        return _seqNum;
    }
    
    /** Sets the sequence number of the persisted buffer. */
    void seqNum(int seqNum)
    {
        _seqNum = seqNum;
    }

    /** Resets a persistence buffer (used when returned to the pool). */
    void reset()
    {
        _isTransmitted = false;
        _tunnelStreamHeaderLen = 0;
    }

    /** Returns the length of the persisted tunnel stream header. */
    int tunnelStreamHeaderLen()
    {
        return _tunnelStreamHeaderLen;
    }
    
    /** Sets the length of the persisted tunnel stream header. */
    void tunnelStreamHeaderLen(int tunnelStreamHeaderLen)
    {
        _tunnelStreamHeaderLen = tunnelStreamHeaderLen;
    }


}
