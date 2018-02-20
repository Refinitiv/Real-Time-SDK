package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.FieldListFlags;
import com.thomsonreuters.upa.codec.LocalFieldSetDefDb;

class FieldListImpl implements FieldList
{
    int         _flags;
    int         _dictionaryId;
    int         _fieldListNum;
    int         _setId;
    final Buffer    _encodedSetData = CodecFactory.createBuffer();
    final Buffer    _encodedEntries = CodecFactory.createBuffer();
	
    @Override
    public void clear()
    {
        _flags = 0;
        _dictionaryId = 0;
        _fieldListNum = 0;
        _setId = 0;
        _encodedSetData.clear();
        _encodedEntries.clear();
    }
	
    @Override
    public int encodeInit(EncodeIterator iter, LocalFieldSetDefDb setDb, int setEncodingMaxSize)
    {
        return Encoders.encodeFieldListInit(iter, this, setDb, setEncodingMaxSize);
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeFieldListComplete(iter, success);
    }

    @Override
    public int decode(DecodeIterator iter, LocalFieldSetDefDb localSetDb)
    {
        return Decoders.decodeFieldList(iter, this, localSetDb);
    }

    @Override
    public String decodeToXml(DecodeIterator iter)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.FIELD_LIST, null, null, null, iter);
    }

    @Override
    public String decodeToXml(DecodeIterator iter, DataDictionary dictionary)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.FIELD_LIST, null, dictionary, null, iter);
    }

    @Override
    public boolean checkHasInfo()
    {
        return ((_flags & FieldListFlags.HAS_FIELD_LIST_INFO) > 0 ? true : false);
    }

    @Override
    public boolean checkHasStandardData()
    {
        return ((_flags & FieldListFlags.HAS_STANDARD_DATA) > 0 ? true : false);
    }

    @Override
    public boolean checkHasSetId()
    {
        return ((_flags & FieldListFlags.HAS_SET_ID) > 0 ? true : false);
    }

    @Override
    public boolean checkHasSetData()
    {
        return ((_flags & FieldListFlags.HAS_SET_DATA) > 0 ? true : false);
    }

    @Override
    public void applyHasInfo()
    {
        _flags = (_flags | FieldListFlags.HAS_FIELD_LIST_INFO);
    }

    @Override
    public void applyHasStandardData()
    {
        _flags = (_flags | FieldListFlags.HAS_STANDARD_DATA);
    }

    @Override
    public void applyHasSetId()
    {
        _flags = (_flags | FieldListFlags.HAS_SET_ID);
    }

    @Override
    public void applyHasSetData()
    {
        _flags = (_flags | FieldListFlags.HAS_SET_DATA);
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
    public int dictionaryId()
    {
        return _dictionaryId;
    }

    @Override
    public void dictionaryId(int dictionaryId)
    {
        assert (dictionaryId >= 0 && dictionaryId <= 32767) : "dictionaryId is out of range (0-32767)"; // 0x8000 (u15rb)

        _dictionaryId = dictionaryId;
    }

    @Override
    public int fieldListNum()
    {
        return _fieldListNum;
    }

    @Override
    public void fieldListNum(int fieldListNum)
    {
        assert (fieldListNum >= -32768 && fieldListNum <= 32767) : "fieldListNum is out of range ((-32768)-32767)"; // int16

        _fieldListNum = fieldListNum;
    }

    @Override
    public int setId()
    {
        return _setId;
    }

    @Override
    public void setId(int setId)
    {
        assert (setId >= 0 && setId <= 32767) : "setId is out of range (0-32767)"; // 0x8000 (u15rb)

        _setId = setId;
    }

    @Override
    public Buffer encodedSetData()
    {
        return _encodedSetData;
    }

    @Override
    public void encodedSetData(Buffer encodedSetData)
    {
        assert (encodedSetData != null) : "encodedSetData must be non-null";

        ((BufferImpl)_encodedSetData).copyReferences(encodedSetData);
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
