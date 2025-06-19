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
import com.refinitiv.eta.codec.Date;
import com.refinitiv.eta.codec.DateTime;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.Decoders;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.MapEntry;
import com.refinitiv.eta.codec.MapEntryFlags;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.Time;
import com.refinitiv.eta.codec.UInt;

class MapEntryImpl implements MapEntry
{
    int         _flags;
    int         _action;
    final Buffer    _permData = CodecFactory.createBuffer();
    final Buffer    _encodedKey = CodecFactory.createBuffer();
    final Buffer    _encodedData = CodecFactory.createBuffer();
	
    @Override
    public void clear()
    {
        _flags = 0;
        _action = 0;
        _permData.clear();
        _encodedKey.clear();
        _encodedData.clear();
    }
	
    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.encodeMapEntry(iter, this, null);
    }

    @Override
    public int encode(EncodeIterator iter, Int keyData)
    {
        return Encoders.encodeMapEntry(iter, this, keyData);
    }

    @Override
    public int encode(EncodeIterator iter, UInt keyData)
    {
        return Encoders.encodeMapEntry(iter, this, keyData);
    }

    @Override
    public int encode(EncodeIterator iter, Real keyData)
    {
        return Encoders.encodeMapEntry(iter, this, keyData);
    }

    @Override
    public int encode(EncodeIterator iter, Date keyData)
    {
        return Encoders.encodeMapEntry(iter, this, keyData);
    }

    @Override
    public int encode(EncodeIterator iter, Time keyData)
    {
        return Encoders.encodeMapEntry(iter, this, keyData);
    }

    @Override
    public int encode(EncodeIterator iter, DateTime keyData)
    {
        return Encoders.encodeMapEntry(iter, this, keyData);
    }

    @Override
    public int encode(EncodeIterator iter, Qos keyData)
    {
        return Encoders.encodeMapEntry(iter, this, keyData);
    }

    @Override
    public int encode(EncodeIterator iter, State keyData)
    {
        return Encoders.encodeMapEntry(iter, this, keyData);
    }

    @Override
    public int encode(EncodeIterator iter, Enum keyData)
    {
        return Encoders.encodeMapEntry(iter, this, keyData);
    }

    @Override
    public int encode(EncodeIterator iter, Buffer keyData)
    {
        return Encoders.encodeMapEntry(iter, this, keyData);
    }

    @Override
    public int encodeInit(EncodeIterator iter, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, null, maxEncodingSize);
    }

    @Override
    public int encodeInit(EncodeIterator iter, Int keyData, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, keyData, maxEncodingSize);
    }

    @Override
    public int encodeInit(EncodeIterator iter, UInt keyData, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, keyData, maxEncodingSize);
    }

    @Override
    public int encodeInit(EncodeIterator iter, Real keyData, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, keyData, maxEncodingSize);
    }

    @Override
    public int encodeInit(EncodeIterator iter, Date keyData, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, keyData, maxEncodingSize);
    }

    @Override
    public int encodeInit(EncodeIterator iter, Time keyData, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, keyData, maxEncodingSize);
    }

    @Override
    public int encodeInit(EncodeIterator iter, DateTime keyData, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, keyData, maxEncodingSize);
    }

    @Override
    public int encodeInit(EncodeIterator iter, Qos keyData, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, keyData, maxEncodingSize);
    }

    @Override
    public int encodeInit(EncodeIterator iter, State keyData, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, keyData, maxEncodingSize);
    }

    @Override
    public int encodeInit(EncodeIterator iter, Enum keyData, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, keyData, maxEncodingSize);
    }

    @Override
    public int encodeInit(EncodeIterator iter, Buffer keyData, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, keyData, maxEncodingSize);
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeMapEntryComplete(iter, this, success);
    }

    @Override
    public int decode(DecodeIterator iter, Object keyData)
    {
        return Decoders.decodeMapEntry(iter, this, keyData);
    }
	
    @Override
    public boolean checkHasPermData()
    {
        return ((_flags & MapEntryFlags.HAS_PERM_DATA) > 0 ? true : false);
    }

    @Override
    public void applyHasPermData()
    {
        _flags = (_flags | MapEntryFlags.HAS_PERM_DATA);
    }

    @Override
    public void action(int action)
    {
        assert (action >= 0 && action <= 15) : "action is out of range (0-15)"; // uint4

        _action = action;
    }

    @Override
    public int action()
    {
        return _action;
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
    public Buffer encodedKey()
    {
        return _encodedKey;
    }

    @Override
    public void encodedKey(Buffer encodedKey)
    {
        assert (encodedKey != null) : "encodedKey must be non-null";

        ((BufferImpl)_encodedKey).copyReferences(encodedKey);
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

    @Override
    public int encode(EncodeIterator iter, Float keyData)
    {
        return Encoders.encodeMapEntry(iter, this, keyData);
    }

    @Override
    public int encode(EncodeIterator iter, Double keyData)
    {
        return Encoders.encodeMapEntry(iter, this, keyData);
    }

    @Override
    public int encodeInit(EncodeIterator iter, Float keyData, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, keyData, maxEncodingSize);
    }

    @Override
    public int encodeInit(EncodeIterator iter, Double keyData, int maxEncodingSize)
    {
        return Encoders.encodeMapEntryInit(iter, this, keyData, maxEncodingSize);
    }
}
