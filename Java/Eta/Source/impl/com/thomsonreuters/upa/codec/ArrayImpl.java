package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;

/* The Array is a uniform primitive type that can contain multiple simple primitive entries. */
class ArrayImpl implements Array
{
    int        _primitiveType;
    int        _itemLength;
    final Buffer _encodedData = CodecFactory.createBuffer();
    boolean    _isBlank;
	    
    @Override
    public void clear()
    {
        _primitiveType = 0;
        _itemLength = 0;
        _encodedData.clear();
    }
	
    void blank()
    {
        clear();
        _isBlank = true;
    }

    @Override
    public boolean isBlank()
    {
        return _isBlank;
    }
    
    @Override
    public int encodeInit(EncodeIterator iter)
    {
        if (!isBlank())
        {
            return Encoders.encodeArrayInit(iter, this);
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
        if (!isBlank())
        {
            return Encoders.encodeArrayComplete(iter, success);
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeArray(iter, this);
    }

    @Override
    public String decodeToXml(DecodeIterator iter)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.ARRAY, null, null, null, iter);
    }

    @Override
    public String decodeToXml(DecodeIterator iter, DataDictionary dictionary)
    {
        return DecodersToXML.decodeDataTypeToXML(DataTypes.ARRAY, null, dictionary, null, iter);
    }

    @Override
    public int primitiveType()
    {
        return _primitiveType;
    }

    @Override
    public void primitiveType(int primitiveType)
    {
        assert (primitiveType > DataTypes.UNKNOWN && primitiveType <= DataTypes.BASE_PRIMITIVE_MAX) :
            "primitiveType must be from the DataTypes enumeration and greater than UNKNOWN and less than or equal to BASE_PRIMITIVE_MAX.";
        assert (primitiveType != DataTypes.ARRAY) : "primitiveType in array can't be DataTypes.ARRAY.";

        _primitiveType = primitiveType;
        _isBlank = false;
    }

    @Override
    public int itemLength()
    {
        return _itemLength;
    }

    @Override
    public void itemLength(int itemLength)
    {
        assert (itemLength >= 0 && itemLength <= 65535) : "itemLength is out of range (0-65535)"; // uint16

        _itemLength = itemLength;
        _isBlank = false;
    }

    @Override
    public Buffer encodedData()
    {
        return _encodedData;
    }
}
