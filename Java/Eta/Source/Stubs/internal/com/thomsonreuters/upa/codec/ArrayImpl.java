package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;

/**
 * The Array is a uniform primitive type that can contain multiple simple primitive entries.
 * @see ClearArray
 * @see DecodeArray
 */
class ArrayImpl implements Array
{
	    
	@Override
    public void clear()
    {
    }
	
	void blank()
	{
	}

	@Override
	public boolean isBlank()
	{
		return true;
	}
    
	@Override
    public int encodeInit(EncodeIterator iter)
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int decode(DecodeIterator iter)
    {
		return CodecReturnCodes.FAILURE;
    }

    @Override
    public String decodeToXml(DecodeIterator iter)
    {
        return null;
    }

    @Override
    public String decodeToXml(DecodeIterator iter, DataDictionary dictionary)
    {
      	return null;
    }

	@Override
	public int primitiveType() 
	{
		return DataTypes.UNKNOWN;
	}

	@Override
	public void primitiveType(int primitiveType) 
	{
        
	}

	@Override
	public int itemLength() 
	{
		return -1;
	}

	@Override
	public void itemLength(int itemLength) 
	{
	    
	}

	@Override
	public Buffer encodedData() 
	{
		return null;
	}
}
