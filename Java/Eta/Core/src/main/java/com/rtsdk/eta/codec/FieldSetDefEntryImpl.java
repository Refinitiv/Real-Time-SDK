package com.rtsdk.eta.codec;

import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.FieldSetDefEntry;

class FieldSetDefEntryImpl implements FieldSetDefEntry
{
    /* The field identifier */
    int     _fieldId;
    
    /* The field data type */
    int     _dataType;
	
    @Override
    public void clear()
    {
        _fieldId = 0;
        _dataType = 0;
    }

    @Override
    public int fieldId()
    {
        return _fieldId;
    }

    @Override
    public void fieldId(int fieldId)
    {
        assert (fieldId >= -32768 && fieldId <= 32767) : "fieldId is out of range (-32768-32767)"; // uint16
        _fieldId = fieldId;
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
