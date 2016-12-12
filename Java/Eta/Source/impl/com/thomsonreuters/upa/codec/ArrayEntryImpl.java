package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;

class ArrayEntryImpl implements ArrayEntry
{
    final Buffer _encodedData = CodecFactory.createBuffer();
	
    @Override
    public void clear()
    {
        _encodedData.clear();
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeArrayEntry(iter, this);
    }

    @Override
    public Buffer encodedData()
    {
        return _encodedData;
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.encodePreencodedArrayEntry(iter, _encodedData);
    }

    @Override
    public int encodeBlank(EncodeIterator iter)
    {
        _encodedData.clear();

        return Encoders.encodePreencodedArrayEntry(iter, _encodedData);
    }

    @Override
    public int encode(EncodeIterator iter, Int data)
    {
        return Encoders.encodeArrayEntry(iter, data);
    }

    @Override
    public int encode(EncodeIterator iter, UInt data)
    {
        return Encoders.encodeArrayEntry(iter, data);
    }

    @Override
    public int encode(EncodeIterator iter, Float data)
    {
        return Encoders.encodeArrayEntry(iter, data);
    }

    @Override
    public int encode(EncodeIterator iter, Double data)
    {
        return Encoders.encodeArrayEntry(iter, data);
    }

    @Override
    public int encode(EncodeIterator iter, Real data)
    {
        return Encoders.encodeArrayEntry(iter, data);
    }

    @Override
    public int encode(EncodeIterator iter, Date data)
    {
        return Encoders.encodeArrayEntry(iter, data);
    }

    @Override
    public int encode(EncodeIterator iter, Time data)
    {
        return Encoders.encodeArrayEntry(iter, data);
    }

    @Override
    public int encode(EncodeIterator iter, DateTime data)
    {
        return Encoders.encodeArrayEntry(iter, data);
    }

    @Override
    public int encode(EncodeIterator iter, Qos data)
    {
        return Encoders.encodeArrayEntry(iter, data);
    }

    @Override
    public int encode(EncodeIterator iter, State data)
    {
        return Encoders.encodeArrayEntry(iter, data);
    }

    @Override
    public int encode(EncodeIterator iter, Enum data)
    {
        return Encoders.encodeArrayEntry(iter, data);
    }

    @Override
    public int encode(EncodeIterator iter, Buffer data)
    {
        if ((data == null) || (data.length() == 0))
        {
            _encodedData.clear();
            return Encoders.encodeArrayEntry(iter, _encodedData);
        }
        return Encoders.encodeArrayEntry(iter, data);
    }

    @Override
    public void encodedData(Buffer encodedData)
    {
        assert (encodedData != null) : "encodedData must be non-null";

        ((BufferImpl)_encodedData).copyReferences(encodedData);
    }
}

