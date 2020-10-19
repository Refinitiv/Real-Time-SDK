package com.refinitiv.eta.transport;

import java.nio.ByteBuffer;

class SocketBuffer extends EtaNode
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
            TransportBufferImpl slice;
            if ((slice = (TransportBufferImpl)super.poll()) != null)
            {
                --_available;
            }
            else
            {
                // create a new slice
                slice = new TransportBufferImpl(this);
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
                SocketBuffer buffer = (SocketBuffer)_poolOwner;
                Pool sbp = buffer._pool;
                if (!sbp._isSharedPoolBuffer)
                {
                    if (!sbp._isProtocolBuffer)
                    {
                        ((RsslSocketChannel)sbp._poolOwner).socketBufferToRecycle(buffer);
                    }
                    else
                    {
                        buffer.returnToPool();
                    }
                }
                else
                {
                    ((ServerImpl)sbp._poolOwner).socketBufferToRecycle(buffer);
                }
            }
        }

        boolean areAllSlicesBack()
        {
            return (_available == _totalSliceCount);
        }
    }

    static final int PACKED_HDR = 2;
    static final int RIPC_WRITE_POSITION = 3;
    static final byte FRAGMENT_RIPC_FLAGS = 0x04;
    static final byte FRAGMENT_HEADER_RIPC_FLAGS = 0x08;
    static final int COMPRESSION_EXTRA_LEN = 16;

    ByteBuffer _dataBuffer;

    // pool of slices
    final SlicesPool _slicesPool;

    int _bytesUsed = 0;
    boolean _isPacked = false;

    SocketBuffer(Pool pool, int size)
    {
        _dataBuffer = ByteBuffer.allocate(size);
        pool(pool);
        _slicesPool = new SlicesPool(this);
    }

    TransportBufferImpl getBufferSlice(int size, boolean packedBuffer)
    {
        // locked by calling method
        TransportBufferImpl slice = null;

        int _headerLength = RIPC_WRITE_POSITION;
        if (!packedBuffer)
        {
            _isPacked = false;
        }
        else
        {
            _headerLength += PACKED_HDR;
            _isPacked = true;
        }

        if ((_dataBuffer.capacity() - _bytesUsed) >= (_headerLength + size))
        {
            slice = (TransportBufferImpl)_slicesPool.poll(); // it returns non null
            slice._data.clear();

            slice._startPosition = _bytesUsed;
            slice._data.limit(_bytesUsed + _headerLength + size);
            slice._data.position(_bytesUsed + _headerLength);
            slice._length = size;
            _bytesUsed = _bytesUsed + _headerLength + size;

            // has to mark the position for the packed message length
            if (_isPacked)
            {
                slice._isPacked = true;
                slice._packedMsgLengthPosition = slice._startPosition + RIPC_WRITE_POSITION;
            }
            else
            {
                slice._isPacked = false;
            }
        }

        return slice;
    }

    TransportBufferImpl getBufferSliceForFragment(int size)
    {
        // locked by calling method
        TransportBufferImpl slice = null;
        _isPacked = false;

        slice = (TransportBufferImpl)_slicesPool.poll();
        slice._startPosition = 0;
        slice._data.limit(size);
        slice._data.position(0);
        _bytesUsed = size;

        return slice;
    }

    void clear()
    {
        _dataBuffer.clear();
        _bytesUsed = 0;
    }

}
