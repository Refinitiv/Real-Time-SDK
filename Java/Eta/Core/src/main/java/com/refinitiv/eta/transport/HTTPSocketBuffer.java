package com.refinitiv.eta.transport;

public class HTTPSocketBuffer extends SocketBuffer
{
    class SlicesPool extends Pool
    {
        SlicesPool(Object o)
        {
            super(o);
        }

        int _available;
        int _totalSliceCount;

        @Override
        EtaNode poll()
        {
            HTTPTransportBufferImpl slice;
            if ((slice = (HTTPTransportBufferImpl)super.poll()) != null)
            {
                --_available;
            }
            else
            {
                slice = new HTTPTransportBufferImpl(this);
                slice._data = _dataBuffer.duplicate();
                ++_totalSliceCount;
            }
            return slice;
        }

        @Override
        void add(EtaNode node)
        {
            super.add(node);
            ++_available;

            if (_available == _totalSliceCount)
            {
                // the SocketBuffer may being recycled
                HTTPSocketBuffer buffer = (HTTPSocketBuffer)_poolOwner;
                Pool sbp = buffer._pool;
                ((RsslSocketChannel)sbp._poolOwner).socketBufferToRecycle(buffer);
            }
        }

        boolean areAllSlicesBack()
        {
            return (_available == _totalSliceCount);
        }
    }

    int http_write_position = 6;
    int http_chunk_end_size = 2;
    final SlicesPool _slicesPool = new SlicesPool(this);

    HTTPSocketBuffer(Pool pool, int size)
    {
        super(pool, size);
    }

    HTTPTransportBufferImpl getBufferSlice(int size, boolean packedBuffer)
    {
        // locked by calling method
        HTTPTransportBufferImpl slice = null;

        int _headerLength = SocketBuffer.RIPC_WRITE_POSITION + http_write_position;
        if (!packedBuffer)
        {
            _isPacked = false;
        }
        else
        {
            _headerLength += SocketBuffer.PACKED_HDR;
            _isPacked = true;
        }

        if ((_dataBuffer.capacity() - _bytesUsed) >= (_headerLength + size))
        {
            slice = (HTTPTransportBufferImpl)_slicesPool.poll(); // it return non null

            slice._data.clear();

            slice._startPosition = _bytesUsed;
            slice._data.limit(_bytesUsed + _headerLength + size + http_chunk_end_size);
            slice._data.position(_bytesUsed + _headerLength);
            slice._length = size;
            _bytesUsed = _bytesUsed + _headerLength + size + http_chunk_end_size;

            // has to mark the position for the packed message length
            if (_isPacked)
            {
                slice._isPacked = true;
                slice._packedMsgOffSetPosition = slice._startPosition + SocketBuffer.RIPC_WRITE_POSITION
                                                 + http_write_position;
            }
            else
            {
                slice._isPacked = false;
            }
        }

        return slice;
    }

    HTTPTransportBufferImpl getBufferSliceForFragment(int size)
    {
        // locked by calling method
        HTTPTransportBufferImpl slice = null;
        _isPacked = false;

        slice = (HTTPTransportBufferImpl)_slicesPool.poll();
        slice._startPosition = 0;
        slice._data.limit(size);
        slice._data.position(0);
        _bytesUsed = size;

        return slice;
    }

}
