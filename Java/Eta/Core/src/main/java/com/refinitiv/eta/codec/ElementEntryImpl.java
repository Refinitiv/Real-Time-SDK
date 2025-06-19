/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.Date;
import com.refinitiv.eta.codec.DateTime;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.Time;
import com.refinitiv.eta.codec.UInt;

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
