package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.ElementSetDefEntry;

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
