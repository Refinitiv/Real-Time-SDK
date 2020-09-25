package com.refinitiv.eta.transport;

import java.nio.ByteBuffer;

/* Implements a JNI buffer that wraps a UPAC RsslBuffer. */
public class JNIBuffer implements TransportBuffer
{
    /* The length of the buffer. */
    int _length;

    /* The position of the data in the ByteBuffer. */
    int _position;

    /* The actual data buffer. */
    ByteBuffer _data;

    int _startPosition;
    boolean _isWriteBuffer;
    boolean _isPacked;

    /* Pointer to the actual rsslBuffer in C. */
    public long _rsslBufferCPtr;

    JNIBuffer()
    {

    }

    JNIBuffer(ByteBuffer data, int length)
    {
        _data = data;
        _length = length;
    }

    public int dataStartPosition()
    {
        return _startPosition;
    }

    @Override
    public ByteBuffer data()
    {
        return _data;
    }

    @Override
    public int length()
    {
        int len = _length; // readBuffer

        if (_isWriteBuffer) // write buffer
        {
            if (_data != null && (_data.position() > _position || _isPacked))
            {
                len = _data.position() - _position;
            }
        }

        return len;
    }

    @Override
    public int copy(ByteBuffer destBuffer)
    {
        if (_length != 0)
        {
            if (destBuffer != null)
            {
                assert (_length <= (destBuffer.limit() - destBuffer.position())) :
                    "destBuffer is too small, use this.length() for proper length";

                if (_length <= (destBuffer.limit() - destBuffer.position()))
                {
                    int origDesPos = destBuffer.position();
                    int origSrcPos = _data.position();
                    int origSrcLim = _data.limit();

                    _data.position(_position);
                    _data.limit(_position + _length);
                    destBuffer.put(_data);
                    _data.limit(origSrcLim); // set limit first, temp limit may be less than orgSrcPos which can cause exception
                    _data.position(origSrcPos);
                    destBuffer.position(origDesPos);

                    return TransportReturnCodes.SUCCESS;
                }
                else
                {
                    return TransportReturnCodes.FAILURE;
                }
            }
            else
            {
                return TransportReturnCodes.FAILURE;
            }
        }
        else
        {
            // length is zero, no copy needed for blank
            return TransportReturnCodes.SUCCESS;
        }
    }
	
    @Override
    public int capacity()
    {
        return _length;
    }
}
