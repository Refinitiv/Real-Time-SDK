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
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.MapFlags;

class MapImpl implements Map
{
    int         _flags;
    int         _keyPrimitiveType;
    int         _keyFieldId;
    int         _containerType = DataTypes.CONTAINER_TYPE_MIN;
    final Buffer    _encodedSetDefs = CodecFactory.createBuffer();
    final Buffer    _encodedSummaryData = CodecFactory.createBuffer();
    int         _totalCountHint;
    final Buffer    _encodedEntries = CodecFactory.createBuffer();
	
    @Override
    public void clear()
    {
        _flags = 0;
        _keyPrimitiveType = 0;
        _keyFieldId = 0;
        _containerType = DataTypes.CONTAINER_TYPE_MIN;
        _encodedSetDefs.clear();
        _encodedSummaryData.clear();
        _totalCountHint = 0;
        _encodedEntries.clear();
    }
	
    @Override
    public int encodeInit(EncodeIterator iter, int summaryMaxSize, int setMaxSize)
    {
        return Encoders.encodeMapInit(iter, this, summaryMaxSize, setMaxSize);
    }

    @Override
    public int encodeSetDefsComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeMapSetDefsComplete(iter, success);
    }

    @Override
    public int encodeSummaryDataComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeMapSummaryDataComplete(iter, this, success);
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeMapComplete(iter, this, success);
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeMap(iter, this);
    }
	
    @Override
    public String decodeToXml(DecodeIterator iter)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.MAP, null, null, null, iter);
    }

    @Override
    public String decodeToXml(DecodeIterator iter, DataDictionary dictionary)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.MAP, null, dictionary, null, iter);
    }

    @Override
    public boolean checkHasSetDefs()
    {
        return (_flags & MapFlags.HAS_SET_DEFS) > 0 ? true : false;
    }

    @Override
    public boolean checkHasSummaryData()
    {
        return ((_flags & MapFlags.HAS_SUMMARY_DATA) > 0 ? true : false);
    }

    @Override
    public boolean checkHasPerEntryPermData()
    {
        return (_flags & MapFlags.HAS_PER_ENTRY_PERM_DATA) > 0 ? true : false;
    }

    @Override
    public boolean checkHasTotalCountHint()
    {
        return ((_flags & MapFlags.HAS_TOTAL_COUNT_HINT) > 0 ? true : false);
    }

    @Override
    public boolean checkHasKeyFieldId()
    {
        return ((_flags & MapFlags.HAS_KEY_FIELD_ID) > 0 ? true : false);
    }

    @Override
    public void applyHasSetDefs()
    {
        _flags = (_flags | MapFlags.HAS_SET_DEFS);
    }

    @Override
    public void applyHasSummaryData()
    {
        _flags = (_flags | MapFlags.HAS_SUMMARY_DATA);
    }

    @Override
    public void applyHasPerEntryPermData()
    {
        _flags = (_flags | MapFlags.HAS_PER_ENTRY_PERM_DATA);
    }

    @Override
    public void applyHasTotalCountHint()
    {
        _flags = (_flags | MapFlags.HAS_TOTAL_COUNT_HINT);
    }

    @Override
    public void applyHasKeyFieldId()
    {
        _flags = (_flags | MapFlags.HAS_KEY_FIELD_ID);
    }

    @Override
    public int flags()
    {
        return _flags;
    }

    @Override
    public void flags(int flags)
    {
        assert (flags >= 0 && flags <= 255) : "flags is out of range (0-255)"; // uint8

        _flags = flags;
    }

    @Override
    public Buffer encodedEntries()
    {
        return _encodedEntries;
    }

    void encodedEntries(Buffer encodedEntries)
    {
        assert (encodedEntries != null) : "encodedEntries must be non-null";

        ((BufferImpl)_encodedEntries).copyReferences(encodedEntries);
    }

    @Override
    public int keyPrimitiveType()
    {
        return _keyPrimitiveType;
    }
	
    @Override
    public void keyPrimitiveType(int keyPrimitiveType)
    {
        assert (keyPrimitiveType > DataTypes.UNKNOWN && keyPrimitiveType <= DataTypes.BASE_PRIMITIVE_MAX) : 
            "keyPrimitiveType must be from the DataTypes enumeration and greater than UNKNOWN and less than or equal to BASE_PRIMITIVE_MAX.";

        _keyPrimitiveType = keyPrimitiveType;
    }

    @Override
    public int keyFieldId()
    {
        return _keyFieldId;
    }

    @Override
    public void keyFieldId(int keyFieldId)
    {
        assert (keyFieldId >= -32768 && keyFieldId <= 32767) : "keyFieldId is out of range ((-32768)-32767)"; // int16

        _keyFieldId = keyFieldId;
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
    public Buffer encodedSetDefs()
    {
        return _encodedSetDefs;
    }

    @Override
    public void encodedSetDefs(Buffer encodedSetDefs)
    {
        assert (encodedSetDefs != null) : "encodedSetDefs must be non-null";

        ((BufferImpl)_encodedSetDefs).copyReferences(encodedSetDefs);
    }

    @Override
    public Buffer encodedSummaryData()
    {
        return _encodedSummaryData;
    }

    @Override
    public void encodedSummaryData(Buffer encodedSummaryData)
    {
        assert (encodedSummaryData != null) : "encodedSummaryData must be non-null";

        ((BufferImpl)_encodedSummaryData).copyReferences(encodedSummaryData);
    }

    @Override
    public int totalCountHint()
    {
        return _totalCountHint;
    }

    @Override
    public void totalCountHint(int totalCountHint)
    {
        assert (totalCountHint >= 0 && totalCountHint <= 1073741823) : 
            "totalCountHint is out of range (0-1073741823)"; // (<  0x40000000) uint30-rb

        _totalCountHint = totalCountHint;
    }
}
