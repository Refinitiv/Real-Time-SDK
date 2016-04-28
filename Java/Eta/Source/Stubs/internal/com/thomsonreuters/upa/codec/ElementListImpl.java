package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.LocalElementSetDefDb;

class ElementListImpl implements ElementList
{
	
	@Override
	public void clear()
	{
	
	}
	
	@Override
	public int encodeInit(EncodeIterator iter, LocalElementSetDefDb setDb, int setEncodingMaxSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeComplete(EncodeIterator iter, boolean success)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int decode(DecodeIterator iter, LocalElementSetDefDb localSetDb)
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
    public void elementListNum(int elementListNum)
    {
            }

    @Override
    public int elementListNum()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public void setId(int setId)
    {
       
    }

    @Override
    public int setId()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public void encodedSetData(Buffer encodedSetData)
    {
       
    }

    @Override
    public Buffer encodedSetData()
    {
        return null;
    }
    
    @Override
    public Buffer encodedEntries()
    {
        return null;
    }

	@Override
	public void flags(int flags) 
	{
			
	}

	@Override
	public int flags() 
	{
		return CodecReturnCodes.FAILURE;
	}
    
   
}
