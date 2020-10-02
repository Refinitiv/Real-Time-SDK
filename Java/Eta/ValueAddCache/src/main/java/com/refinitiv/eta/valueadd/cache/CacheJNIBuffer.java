package com.refinitiv.eta.valueadd.cache;

import java.nio.ByteBuffer;

import com.refinitiv.eta.valueadd.common.VaNode;

class CacheJNIBuffer extends VaNode
{
    int _length;
    int _capability;
    ByteBuffer _data;

    long _etaBufferCPtr = 0;

    void data(ByteBuffer data, int length)
    {
        _data = data;
        _length = length;
        _capability = length;
    }

    void clear()
    {
        if (_data != null)
            _data.clear();

        _length = 0;
    }

}