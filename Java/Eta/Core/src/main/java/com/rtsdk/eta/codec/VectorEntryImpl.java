package com.rtsdk.eta.codec;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.VectorEntry;
import com.rtsdk.eta.codec.VectorEntryFlags;

class VectorEntryImpl implements VectorEntry
{
    final int MAX_INDEX = 0x3fffffff;
    int         _flags;
    int         _action;
    long        _index;
    final Buffer  _permData = CodecFactory.createBuffer();
    final Buffer  _encodedData = CodecFactory.createBuffer();

    @Override
    public void clear()
    {
        _flags = 0;
        _action = 0;
        _index = 0;
        _permData.clear();
        _encodedData.clear();
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.encodeVectorEntry(iter, this);
    }

    @Override
    public int encodeInit(EncodeIterator iter, int maxEncodingSize)
    {
        return Encoders.encodeVectorEntryInit(iter, this, maxEncodingSize);
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeVectorEntryComplete(iter, success);
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeVectorEntry(iter, this);
    }

    @Override
    public boolean checkHasPermData()
    {
        return (_flags & VectorEntryFlags.HAS_PERM_DATA) > 0 ? true : false;
    }

    @Override
    public void applyHasPermData()
    {
        _flags |= VectorEntryFlags.HAS_PERM_DATA;
    }

    @Override
    public int flags()
    {
        return _flags;
    }

    @Override
    public void flags(int flags)
    {
        assert (flags >= 0 && flags <= 15) : "flags is out of range (0-15)"; // uint4

        _flags = flags;
    }

    @Override
    public int action()
    {
        return _action;
    }

    @Override
    public void action(int action)
    {
        assert (action >= 0 && action <= 15) : "action is out of range (0-15)"; // uint4

        _action = action;
    }

    @Override
    public long index()
    {
        return _index;
    }

    @Override
    public void index(long index)
    {
        assert (index >= 0 && index <= 1073741823) : "index is out of range (0-1073741823)"; // (<0x40000000) uint30-rb

        _index = index;
    }

    @Override
    public Buffer permData()
    {
        return _permData;
    }

    @Override
    public void permData(Buffer permData)
    {
        assert (permData != null) : "perData must be non-null";

        ((BufferImpl)_permData).copyReferences(permData);
    }

    @Override
    public Buffer encodedData()
    {
        return _encodedData;
    }

    @Override
    public void encodedData(Buffer encodedData)
    {
        assert (encodedData != null) : "encodedData must be non-null";

        ((BufferImpl)_encodedData).copyReferences(encodedData);
    }
}
