package com.rtsdk.eta.codec;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.DataDictionary;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Vector;
import com.rtsdk.eta.codec.VectorFlags;

class VectorImpl implements Vector
{
    int         _flags;
    int         _containerType = DataTypes.CONTAINER_TYPE_MIN;
    final Buffer    _encodedSetDefs = CodecFactory.createBuffer();
    final Buffer    _encodedSummaryData = CodecFactory.createBuffer();
    int         _totalCountHint;
    final Buffer    _encodedEntries = CodecFactory.createBuffer();

    @Override
    public void clear()
    {
        _flags = 0;
        _containerType = DataTypes.CONTAINER_TYPE_MIN;
        _encodedSetDefs.clear();
        _encodedSummaryData.clear();
        _totalCountHint = 0;
        _encodedEntries.clear();
    }

    @Override
    public int encodeInit(EncodeIterator iter, int summaryMaxSize, int setMaxSize)
    {
        return Encoders.encodeVectorInit(iter, this, summaryMaxSize, setMaxSize);
    }

    @Override
    public int encodeSetDefsComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeVectorSetDefsComplete(iter, success);
    }

    @Override
    public int encodeSummaryDataComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeVectorSummaryDataComplete(iter, this, success);
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeVectorComplete(iter, success, this);
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeVector(iter, this);
    }

    @Override
    public String decodeToXml(DecodeIterator iter)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.VECTOR, null, null, null, iter);
    }

    @Override
    public String decodeToXml(DecodeIterator iter, DataDictionary dictionary)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.VECTOR, null, dictionary, null, iter);
    }

    @Override
    public boolean checkHasSetDefs()
    {
        return ((_flags & VectorFlags.HAS_SET_DEFS) > 0 ? true : false);
    }

    @Override
    public boolean checkHasSummaryData()
    {
        return ((_flags & VectorFlags.HAS_SUMMARY_DATA) > 0 ? true : false);
    }

    @Override
    public boolean checkHasPerEntryPermData()
    {
        return ((_flags & VectorFlags.HAS_PER_ENTRY_PERM_DATA) > 0 ? true : false);
    }

    @Override
    public boolean checkHasTotalCountHint()
    {
        return ((_flags & VectorFlags.HAS_TOTAL_COUNT_HINT) > 0 ? true : false);
    }

    @Override
    public boolean checkSupportsSorting()
    {
        return (_flags & VectorFlags.SUPPORTS_SORTING) > 0 ? true : false;
    }

    @Override
    public void applyHasSetDefs()
    {
        _flags = (_flags | VectorFlags.HAS_SET_DEFS);
    }

    @Override
    public void applyHasSummaryData()
    {
        _flags = (_flags | VectorFlags.HAS_SUMMARY_DATA);
    }

    @Override
    public void applyHasPerEntryPermData()
    {
        _flags = (_flags | VectorFlags.HAS_PER_ENTRY_PERM_DATA);
    }

    @Override
    public void applyHasTotalCountHint()
    {
        _flags = (_flags | VectorFlags.HAS_TOTAL_COUNT_HINT);
    }

    @Override
    public void applySupportsSorting()
    {
        _flags = (_flags | VectorFlags.SUPPORTS_SORTING);
    }

    @Override
    public int flags()
    {
        return _flags;
    }

    @Override
    public void flags(int flags)
    {
        assert (flags >= 0 && flags <= 255) : "flags is out of range (0-255)"; // unit8

        _flags = flags;
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
            "totalCountHint is out of range (0-1073741823)"; // (<0x40000000) uint30-rb

        _totalCountHint = totalCountHint;
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
}
