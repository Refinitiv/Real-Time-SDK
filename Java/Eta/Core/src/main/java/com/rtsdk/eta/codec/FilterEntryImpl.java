package com.rtsdk.eta.codec;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.FilterEntry;
import com.rtsdk.eta.codec.FilterEntryFlags;

class FilterEntryImpl implements FilterEntry
{
    int         _flags;
    int         _action;
    int         _id;
    int         _containerType = DataTypes.CONTAINER_TYPE_MIN;
    final Buffer    _permData = CodecFactory.createBuffer();
    final Buffer    _encodedData = CodecFactory.createBuffer();
    
    @Override
    public void clear()
    {
        _flags = 0;
        _action = 0;
        _id = 0;
        _containerType = DataTypes.CONTAINER_TYPE_MIN;
        _permData.clear();
        _encodedData.clear();
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.encodeFilterEntry(iter, this);
    }

    @Override
    public int encodeInit(EncodeIterator iter, int maxEncodingSize)
    {
        return Encoders.encodeFilterEntryInit(iter, this, maxEncodingSize);
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeFilterEntryComplete(iter, success);
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeFilterEntry(iter, this);
    }

    @Override
    public boolean checkHasPermData()
    {
        return (_flags & FilterEntryFlags.HAS_PERM_DATA) > 0 ? true : false;
    }

    @Override
    public boolean checkHasContainerType()
    {
        return ((_flags & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0 ? true : false);
    }

    @Override
    public void applyHasPermData()
    {
        _flags = (_flags | FilterEntryFlags.HAS_PERM_DATA);
    }

    @Override
    public void applyHasContainerType()
    {
        _flags = (_flags | FilterEntryFlags.HAS_CONTAINER_TYPE);
    }

    @Override
    public int flags()
    {
        return _flags;
    }

    @Override
    public void flags(int flags)
    {
        assert (flags >= 0 && flags <= 15) : "flags is out of range (0-15)"; // 0x0F (uint4)

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
        assert (action >= 0 && action <= 15); // 0x0F (uint4)

        _action = action;
    }

    @Override
    public int id()
    {
        return _id;
    }

    @Override
    public void id(int id)
    {
        assert (id >= 0 && id <= 255) : "id is out of range (0-255)"; // uint8
        _id = id;
    }

    @Override
    public int containerType()
    {
        return _containerType;
    }

    @Override
    public void containerType(int containerType)
    {
        assert (containerType >= DataTypes.CONTAINER_TYPE_MIN && containerType <= DataTypes.LAST) :
            "containerType must be from the DataTypes enumeration in the range CONTAINER_TYPE_MIN to LAST.";

        _containerType = containerType;
    }

    @Override
    public Buffer permData()
    {
        return _permData;
    }

    @Override
    public void permData(Buffer permData)
    {
        assert (permData != null) : "permData must be non-null";

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
