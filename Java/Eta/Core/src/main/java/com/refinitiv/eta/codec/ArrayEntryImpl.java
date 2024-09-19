/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.ArrayEntry;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.Date;
import com.refinitiv.eta.codec.DateTime;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.Time;
import com.refinitiv.eta.codec.UInt;

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

