package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Series;

class SeriesImpl implements Series
{
	
	@Override
	public void clear()
	{
	
	}
	
	@Override
	public int encodeInit(EncodeIterator iter, int summaryMaxSize, int setMaxSize)
	{
		return CodecReturnCodes.FAILURE;
	}		        

	@Override
	public int encodeSetDefsComplete(EncodeIterator iter, boolean success)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeSummaryDataComplete(EncodeIterator iter, boolean success)
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
	public boolean checkHasSetDefs()
	{
		return false;
	}

	@Override
	public boolean checkHasSummaryData()
	{
		return false;
	}

	@Override
	public boolean checkHasTotalCountHint()
	{
		return false;
	}

	@Override
	public void applyHasSetDefs()
	{
		
	}

	@Override
	public void applyHasSummaryData()
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
	public Buffer encodedSetDefs() 
	{
		return null;
	}

	@Override
	public void encodedSetDefs(Buffer encodedSetDefs) 
	{
	  
	}

	@Override
	public Buffer encodedSummaryData() 
	{
		return null;
	}

	@Override
	public void encodedSummaryData(Buffer encodedSummaryData) 
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

	//@Override
	void encodedEntries(Buffer encodedEntries) 
	{
	   
	}
}
