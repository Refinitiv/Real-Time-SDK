package com.refinitiv.eta.valueadd.reactor;

import java.nio.ByteBuffer;

class SlicedBufferPool
{
    static final int TUNNEL_STREAM_HDR_SIZE = 128;
    
    SliceableBufferList _bufferList = new SliceableBufferList();
    SliceableBufferList _userBufferList = new SliceableBufferList();
    SliceableBuffer _currentBuffer, _currentUserBuffer;
    int _maxMsgSize;
    int _maxNumUserBuffers;
    int _currentNumUserBuffers;
    int _currentNumUserBuffersByApplication;
    
    SlicedBufferPool(int maxMsgSize, int numBuffers)
    {
        _maxMsgSize = maxMsgSize;
        _maxNumUserBuffers = numBuffers;
    }
    
    class DuplicateBuffer
    {
        ByteBuffer _data; // duplicate ByteBuffer
        DuplicateBuffer _next; // used in DuplicateList
    }
    
    class SliceableBuffer
    {
        ByteBuffer _data;
        
        SliceableBuffer()
        {
            _data = ByteBuffer.allocate(_maxMsgSize + TUNNEL_STREAM_HDR_SIZE);
        }
        
        int _position;
        int _numSlicesInUse;
        SliceableBuffer _next; // used in SliceableBufferList
        DuplicateList _duplicateList = new DuplicateList();

        // gets a duplicate from the duplicate list
        // or creates one if none in list
        DuplicateBuffer getDuplicate(boolean isUserBuffer)
        {
            DuplicateBuffer duplicateBuffer = _duplicateList.pop();
            if (duplicateBuffer != null)
            {
                duplicateBuffer._data.clear();                
            }
            else
            {
                duplicateBuffer = new DuplicateBuffer();
                if (!isUserBuffer)
                {
                    duplicateBuffer._data = _currentBuffer._data.duplicate();
                }
                else
                {
                    duplicateBuffer._data = _currentUserBuffer._data.duplicate();
                }
            }
            
            return duplicateBuffer;
        }
        
        // returns the duplicate to the duplicate list
        void releaseDuplicate(TunnelStreamBuffer bufferImpl)
        {
            _duplicateList.push(bufferImpl._duplicateBuffer);
        }
    }

    int count()
	{
		return _bufferList.count();
	}
    
	// list of sliceable buffers
    class SliceableBufferList
    {
        SliceableBuffer _head;
        SliceableBuffer _tail;
        private int _count;
        
        void push(SliceableBuffer buffer)
        {
            if (_tail != null)
            {
                buffer._next = null;
                _tail._next = buffer;
                _tail = buffer;
            }
            else
            {
                _head = _tail = buffer;
                _head._next = _tail._next = null;
            }
            _count++;
        }
        
        SliceableBuffer pop()
        {
            SliceableBuffer retBuffer = null;
            
            if (_head != null)
            {
                retBuffer = _head;
                _head = retBuffer._next;
                if (_head == null)
                {
                    _tail = null;
                }
                _count--;
            }

            return retBuffer;
        }
        
        int count()
        {
            return _count;
        }
    }

    // list of duplicate ByteBuffers
    class DuplicateList
    {
        DuplicateBuffer _head;
        DuplicateBuffer _tail;
        private int _count;
        
        void push(DuplicateBuffer buffer)
        {
            if (_tail != null)
            {
                buffer._next = null;
                _tail._next = buffer;
                _tail = buffer;
            }
            else
            {
                _head = _tail = buffer;
                _head._next = _tail._next = null;
            }
            _count++;
        }
        
        DuplicateBuffer pop()
        {
            DuplicateBuffer retBuffer = null;
            
            if (_head != null)
            {
                retBuffer = _head;
                _head = retBuffer._next;
                if (_head == null)
                {
                    _tail = null;
                }
                _count--;
            }

            return retBuffer;
        }
        
        int count()
        {
            return _count;
        }
    }
    
    // gets a buffer slice
    void getBufferSlice(TunnelStreamBuffer bufferImpl, int length, boolean isForUser)
    {
        if (!isForUser)
        {
            getBufferSlice(bufferImpl, length);
        }
        else
        {
            getUserBufferSlice(bufferImpl, length);
        }
    }
    
    // gets a buffers slice for internal use
	void getBufferSlice(TunnelStreamBuffer bufferImpl, int length)
	{
        if (_currentBuffer == null || (_currentBuffer._data.limit() - _currentBuffer._position) < length)
        {
            _currentBuffer = _bufferList.pop();
            if (_currentBuffer == null)
            {
                _currentBuffer = new SliceableBuffer();
            }
        }

        DuplicateBuffer duplicateBuffer = _currentBuffer.getDuplicate(false);
        bufferImpl._duplicateBuffer = duplicateBuffer;
        bufferImpl.data(duplicateBuffer._data, _currentBuffer._position, length);
        
        _currentBuffer._position += length;
        _currentBuffer._numSlicesInUse++;
        bufferImpl._parentBuffer = _currentBuffer;
        bufferImpl._isUserBuffer = false;
	}

    // gets a buffers slice for use by the tunnel stream
    void getUserBufferSlice(TunnelStreamBuffer bufferImpl, int length)
    {
        boolean maxUserBuffersReached = false;

        if (_currentUserBuffer != null && _currentUserBuffer._position==0)
            _currentNumUserBuffersByApplication++;

        if (_currentUserBuffer == null || (_currentUserBuffer._data.limit() - _currentUserBuffer._position) < length)
        {
            _currentUserBuffer = _userBufferList.pop();

            if (_currentUserBuffer != null && _currentUserBuffer._position==0)
                _currentNumUserBuffersByApplication++;

            if (_currentUserBuffer == null || (_currentUserBuffer._data.limit() - _currentUserBuffer._position) < length)
            {
                if (_currentNumUserBuffers < _maxNumUserBuffers)
                {
                    _currentUserBuffer = new SliceableBuffer();
                    _currentNumUserBuffers++;
                    _currentNumUserBuffersByApplication++;
                }
                else
                {
                    maxUserBuffersReached = true;
                }
            }
        }

        if (!maxUserBuffersReached)
        {
            DuplicateBuffer duplicateBuffer = _currentUserBuffer.getDuplicate(true);
            bufferImpl._duplicateBuffer = duplicateBuffer;
            duplicateBuffer._data.position(_currentUserBuffer._position);
            bufferImpl.data(duplicateBuffer._data, _currentUserBuffer._position, length);
            
            _currentUserBuffer._position += length;
            _currentUserBuffer._numSlicesInUse++;
            bufferImpl._parentBuffer = _currentUserBuffer;
            bufferImpl._isUserBuffer = true;
        }
        else
        {
            if (bufferImpl != null)
            {
                bufferImpl.data(null);
                if (bufferImpl._parentBuffer != null)
                {
                    bufferImpl._parentBuffer._position = 0;
                    bufferImpl._parentBuffer._data.clear();
                    bufferImpl._parentBuffer = null;
                }
            }
        }
    }

    // releases a buffer slice back to the duplicate list
	// and back to the sliceable buffer list if all slices have been released
	void releaseBufferSlice(TunnelStreamBuffer bufferImpl)
	{
        SliceableBuffer parentBuffer = bufferImpl._parentBuffer;
        parentBuffer.releaseDuplicate(bufferImpl);
        if (--parentBuffer._numSlicesInUse == 0)
        {
            // reset appropriate fields for reuse
            parentBuffer._position = 0;
            parentBuffer._data.clear();
            if (!bufferImpl._isUserBuffer && parentBuffer != _currentBuffer)
            {
                _bufferList.push(parentBuffer);
            }
            else if (bufferImpl._isUserBuffer && parentBuffer != _currentUserBuffer)
            {
                _userBufferList.push(parentBuffer);
            }

            if (bufferImpl._isUserBuffer)
                _currentNumUserBuffersByApplication--;
        }
    }

    int getBuffersUsed() {
        return _currentNumUserBuffersByApplication;
    }
}

