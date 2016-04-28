package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.LocalFieldSetDefDb;

class FieldListImpl implements FieldList
{

	@Override
	public void clear()
	{
		
	}
	
	@Override
	public int encodeInit(EncodeIterator iter, LocalFieldSetDefDb setDb, int setEncodingMaxSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeComplete(EncodeIterator iter, boolean success)
	{
		return CodecReturnCodes.FAILURE;
	}
	
	@Override
	public int decode(DecodeIterator iter, LocalFieldSetDefDb localSetDb)
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
	public boolean checkHasInfo()
	{
		return false;
	}

	@Override
	public boolean checkHasStandardData()
	{
		return false;
	}

	@Override
	public boolean checkHasSetId()
	{
		return false;
	}

	@Override
	public boolean checkHasSetData()
	{
		return false;
	}

	@Override
	public void applyHasInfo()
	{
	
	}

	@Override
	public void applyHasStandardData()
	{
	
	}

	@Override
	public void applyHasSetId()
	{
	
	}

	@Override
	public void applyHasSetData()
	{
	
	}

	@Override
	public int flags() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public void flags(int flags) 
	{
	    
	}

	@Override
	public int dictionaryId() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public void dictionaryId(int dictionaryId) 
	{
	   
	}

	@Override
	public int fieldListNum() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public void fieldListNum(int fieldListNum) 
	{
		
	}

	@Override
	public int setId() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public void setId(int setId) 
	{
	  
	}

	@Override
	public Buffer encodedSetData() 
	{
		return null;
	}

	@Override
	public void encodedSetData(Buffer encodedSetData) 
	{
	   
	}

	@Override
	public Buffer encodedEntries() 
	{
		return null;
	}

	
}
