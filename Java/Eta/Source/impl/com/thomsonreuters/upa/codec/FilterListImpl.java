package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FilterList;
import com.thomsonreuters.upa.codec.FilterListFlags;

class FilterListImpl implements FilterList
{
    int         _flags;
    int         _containerType = DataTypes.CONTAINER_TYPE_MIN;
    int         _totalCountHint;
    final Buffer    _encodedEntries = CodecFactory.createBuffer();
	
    @Override
    public void clear()
    {
        _flags = 0;
        _containerType = DataTypes.CONTAINER_TYPE_MIN;
        _totalCountHint = 0;
        _encodedEntries.clear();
    }
	
    @Override
    public int encodeInit(EncodeIterator iter)
    {
        return Encoders.encodeFilterListInit(iter, this);
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeFilterListComplete(iter, success, this);
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeFilterList(iter, this);
    }

    @Override
    public String decodeToXml(DecodeIterator iter)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.FILTER_LIST, null, null, null, iter);
    }

    @Override
    public String decodeToXml(DecodeIterator iter, DataDictionary dictionary)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.FILTER_LIST, null, dictionary, null, iter);
    }

    @Override
    public boolean checkHasPerEntryPermData()
    {
        return ((_flags & FilterListFlags.HAS_PER_ENTRY_PERM_DATA) > 0 ? true : false);
    }

    @Override
    public boolean checkHasTotalCountHint()
    {
        return ((_flags & FilterListFlags.HAS_TOTAL_COUNT_HINT) > 0 ? true : false);
    }

    @Override
    public void applyHasPerEntryPermData()
    {
        _flags = (_flags | FilterListFlags.HAS_PER_ENTRY_PERM_DATA);
    }

    @Override
    public void applyHasTotalCountHint()
    {
        _flags = (_flags | FilterListFlags.HAS_TOTAL_COUNT_HINT);
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
    public int totalCountHint()
    {
        return _totalCountHint;
    }

    @Override
    public void totalCountHint(int totalCountHint)
    {
        assert (totalCountHint >= 0 && totalCountHint <= 255) : "totalCountHint is out of range (0-255)"; // uint8

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
