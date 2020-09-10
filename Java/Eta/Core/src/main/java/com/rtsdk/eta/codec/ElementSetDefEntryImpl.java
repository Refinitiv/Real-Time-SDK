package com.rtsdk.eta.codec;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.ElementSetDefEntry;

class ElementSetDefEntryImpl implements ElementSetDefEntry
{
    final Buffer   _name = CodecFactory.createBuffer();
    int         _dataType;
	
    @Override
    public void clear()
    {
        _name.clear();
        _dataType = 0;
    }

    @Override
    public Buffer name()
    {
        return _name;
    }

    @Override
    public void name(Buffer name)
    {
        assert (name != null) : "name must be non-null";

        ((BufferImpl)_name).copyReferences(name);
    }

    @Override
    public int dataType()
    {
        return _dataType;
    }

    @Override
    public void dataType(int dataType)
    {
        assert (dataType > DataTypes.UNKNOWN && dataType <= DataTypes.LAST) : "dataType is out of range. Refer to DataTypes";

        _dataType = dataType;
    }
}
