package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;

class ElementEntryImpl implements ElementEntry
{
    final Buffer    _name = CodecFactory.createBuffer();
    int          _dataType;
    final Buffer    _encodedData = CodecFactory.createBuffer();

    @Override
    public void clear()
    {
        _name.clear();
        _dataType = 0;
        _encodedData.clear();
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.encodeElementEntry(iter, this, null);
    }

    @Override
    public int encodeBlank(EncodeIterator iter)
    {
        _encodedData.clear();

        return Encoders.encodeElementEntry(iter, this, null);
    }

    @Override
    public int encode(EncodeIterator iter, Int data)
    {
        return Encoders.encodeElementEntry(iter, this, data);
    }

    @Override
    public int encode(EncodeIterator iter, UInt data)
    {
        return Encoders.encodeElementEntry(iter, this, data);
    }

    @Override
    public int encode(EncodeIterator iter, Real data)
    {
        return Encoders.encodeElementEntry(iter, this, data);
    }

    @Override
    public int encode(EncodeIterator iter, Date data)
    {
        return Encoders.encodeElementEntry(iter, this, data);
    }

    @Override
    public int encode(EncodeIterator iter, Time data)
    {
        return Encoders.encodeElementEntry(iter, this, data);
    }

    @Override
    public int encode(EncodeIterator iter, DateTime data)
    {
        return Encoders.encodeElementEntry(iter, this, data);
    }

    @Override
    public int encode(EncodeIterator iter, Qos data)
    {
        return Encoders.encodeElementEntry(iter, this, data);
    }

    @Override
    public int encode(EncodeIterator iter, State data)
    {
        return Encoders.encodeElementEntry(iter, this, data);
    }

    @Override
    public int encode(EncodeIterator iter, Enum data)
    {
        return Encoders.encodeElementEntry(iter, this, data);
    }

    @Override
    public int encode(EncodeIterator iter, Buffer data)
    {
        return Encoders.encodeElementEntry(iter, this, data);
    }
	
    @Override
    public int encodeInit(EncodeIterator iter, int encodingMaxSize)
    {
        return Encoders.encodeElementEntryInit(iter, this, encodingMaxSize);
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeElementEntryComplete(iter, success);
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeElementEntry(iter, this);
    }

    @Override
    public void name(Buffer name)
    {
        assert (name != null) : "name must be non-null";

        ((BufferImpl)_name).copyReferences(name);
    }

    @Override
    public Buffer name()
    {
        return _name;
    }
	
    @Override
    public void dataType(int dataType)
    {
        assert (dataType > DataTypes.UNKNOWN && dataType <= DataTypes.LAST) : "dataType is out of range. Refer to DataTypes";

        _dataType = dataType;
    }

    @Override
    public int dataType()
    {
        return _dataType;
    }

    @Override
    public void encodedData(Buffer encodedData)
    {
        assert (encodedData != null) : "encodedData must be non-null";

        ((BufferImpl)_encodedData).copyReferences(encodedData);
    }

    @Override
    public Buffer encodedData()
    {
        return _encodedData;
    }

    @Override
    public int encode(EncodeIterator iter, Float data)
    {
        return Encoders.encodeElementEntry(iter, this, data);
    }

    @Override
    public int encode(EncodeIterator iter, Double data)
    {
        return Encoders.encodeElementEntry(iter, this, data);
    }
}
