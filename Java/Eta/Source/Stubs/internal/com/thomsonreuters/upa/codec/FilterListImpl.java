package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FilterList;

class FilterListImpl implements FilterList
{
	
	@Override
	public void clear()
	{
		
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
	public boolean checkHasPerEntryPermData()
	{
		return false;
	}

	@Override
	public boolean checkHasTotalCountHint()
	{
		return false;
	}

	@Override
	public void applyHasPerEntryPermData()
	{
		
	}

	@Override
	public void applyHasTotalCountHint()
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
	public int containerType() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public void containerType(int containerType) 
	{
	   
	}

	@Override
	public int totalCountHint() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public void totalCountHint(int totalCountHint) 
	{
	   
	}

	@Override
	public Buffer encodedEntries() 
	{
		return null;
	}

	
}
