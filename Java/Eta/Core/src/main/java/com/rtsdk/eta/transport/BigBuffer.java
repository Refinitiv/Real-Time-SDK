package com.rtsdk.eta.transport;

import java.nio.ByteBuffer;

class BigBuffer extends HTTPTransportBufferImpl
{
    static int _ID;

    TransportBufferImpl _firstBuffer;
    short _fragmentId;
    boolean _isWritePaused = false;

    BigBuffer(Pool pool, int size)
    {
        super(pool, size);
        _isWriteBuffer = true;
    }

    @Override
    boolean isBigBuffer()
    {
        return true;
    }
	
    /* This method will set the ripcVersion for the BigBuffer
     * and adjust the first and next fragment header length. */
    static void ripcVersion(int ripcVersion)
    {
        _ripcVersion = ripcVersion;
        if (_ripcVersion >= Ripc.RipcVersions.VERSION13)
        {
            // two byte fragId
            _firstFragmentHeaderLength = 10;
            _nextFragmentHeaderLength = 6;
        }
        else
        {
            // one byte fragId
            _firstFragmentHeaderLength = 9;
            _nextFragmentHeaderLength = 5;
        }
    }

    @Override
    void returnToPool()
    {
        if (!_inPool)
        {
            if (_firstBuffer != null)
                _firstBuffer.returnToPool();
            _firstBuffer = null;
            _isWritePaused = false;
            _data.position(0);
            _data.limit(_data.capacity());
            _pool.add(this);
        }
    }

    void id()
    {
        _fragmentId = (short)++_ID;
    	if (_fragmentId == 0)
    	{
    		_fragmentId = 1;
    		_ID = 1;
    	}
    }

    short fragmentId()
    {
        return _fragmentId;
    }

    @Override
    public int length()
    {
        int len = _length; // default to buffer capacity if nothing yet written

        if (_data.position() - _startPosition > 0)
        {
            len = _data.position() - _startPosition;
        }

        return len;
    }

    /* Returns the RipcVersions. */
    int ripcVersion()
    {
        return _ripcVersion;
    }

    @Override
    public int copy(ByteBuffer destBuffer)
    {
        int retVal = TransportReturnCodes.SUCCESS;
        int len = 0;
        int srcStartPos = 0, destStartPos = destBuffer.position();

        try
        {
            len = _data.position() - _startPosition;
            srcStartPos = _startPosition;
            // copy contents
            for (int i = 0; i < len; i++)
            {
                destBuffer.put(destStartPos + i, _data.get(srcStartPos + i));
            }
        }
        catch (Exception e)
        {
            retVal = TransportReturnCodes.FAILURE;
        }

        return retVal;
    }

    @Override
    public int dataStartPosition()
    {
        return _startPosition;
    }

}

