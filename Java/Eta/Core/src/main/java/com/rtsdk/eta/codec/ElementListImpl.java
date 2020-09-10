package com.rtsdk.eta.codec;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.DataDictionary;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.ElementList;
import com.rtsdk.eta.codec.ElementListFlags;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.LocalElementSetDefDb;

class ElementListImpl implements ElementList
{
    int         _flags;
    int         _elementListNum;
    int         _setId;
    final Buffer    _encodedSetData = CodecFactory.createBuffer();
    final Buffer    _encodedEntries = CodecFactory.createBuffer();

    @Override
    public void clear()
    {
        _flags = 0;
        _elementListNum = 0;
        _setId = 0;
        _encodedSetData.clear();
        _encodedEntries.clear();
    }
	
    @Override
    public int encodeInit(EncodeIterator iter, LocalElementSetDefDb setDb, int setEncodingMaxSize)
    {
        return Encoders.encodeElementListInit(iter, this, setDb, setEncodingMaxSize);
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeElementListComplete(iter, success);
    }

    @Override
    public int decode(DecodeIterator iter, LocalElementSetDefDb localSetDb)
    {
        return Decoders.decodeElementList(iter, this, localSetDb);
    }

    @Override
    public String decodeToXml(DecodeIterator iter)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.ELEMENT_LIST, null, null, null, iter);
    }

    @Override
    public String decodeToXml(DecodeIterator iter, DataDictionary dictionary)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.ELEMENT_LIST, null, dictionary, null, iter);
    }

    @Override
    public boolean checkHasInfo()
    {
        return ((_flags & ElementListFlags.HAS_ELEMENT_LIST_INFO) > 0 ? true : false);
    }

    @Override
    public boolean checkHasStandardData()
    {
        return ((_flags & ElementListFlags.HAS_STANDARD_DATA) > 0 ? true : false);
    }

    @Override
    public boolean checkHasSetId()
    {
        return ((_flags & ElementListFlags.HAS_SET_ID) > 0 ? true : false);
    }

    @Override
    public boolean checkHasSetData()
    {
        return ((_flags & ElementListFlags.HAS_SET_DATA) > 0 ? true : false);
    }

    @Override
    public void applyHasInfo()
    {
        _flags |= ElementListFlags.HAS_ELEMENT_LIST_INFO;
    }

    @Override
    public void applyHasStandardData()
    {
        _flags |= ElementListFlags.HAS_STANDARD_DATA;
    }

    @Override
    public void applyHasSetId()
    {
        _flags |= ElementListFlags.HAS_SET_ID;
    }

    @Override
    public void applyHasSetData()
    {
        _flags |= ElementListFlags.HAS_SET_DATA;
    }

    @Override
    public void elementListNum(int elementListNum)
    {
        assert (elementListNum >= -32768 && elementListNum <= 32767) : "elementListNum is out of range ((-32768)-32767)"; // int16

        _elementListNum = elementListNum;
    }

    @Override
    public int elementListNum()
    {
        return _elementListNum;
    }

    @Override
    public void setId(int setId)
    {
        assert (setId >= 0 && setId <= 32767) : "setId is out of range (0-32767)"; // 0x8000 (u15rb)

        _setId = setId;
    }

    @Override
    public int setId()
    {
        return _setId;
    }

    @Override
    public void encodedSetData(Buffer encodedSetData)
    {
        assert (encodedSetData != null) : "encodedSetData must be non-null";

        ((BufferImpl)_encodedSetData).copyReferences(encodedSetData);
    }

    @Override
    public Buffer encodedSetData()
    {
        return _encodedSetData;
    }    

    void encodedEntries(Buffer encodedEntries)
    {
        assert (encodedEntries != null) : "encodedEntries must be non-null";

        ((BufferImpl)_encodedEntries).copyReferences(encodedEntries);
    }

    @Override
    public Buffer encodedEntries()
    {
        return _encodedEntries;
    }
    
    /* Sets all the flags applicable to this element list
     * 
     * flags ia an integer containing all the flags applicable to this element list
     */
    public void flags(int flags)
    {
        assert (flags >= 0 && flags <= 255) : "flags is out of range (0-255)"; // uint8

        _flags = flags;
    }
    
    /* Returns all the flags applicable to this element list */
    public int flags()
    {
        return _flags;
    }
}
