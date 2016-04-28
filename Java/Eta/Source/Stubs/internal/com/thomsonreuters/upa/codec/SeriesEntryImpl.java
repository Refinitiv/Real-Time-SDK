package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.SeriesEntry;

class SeriesEntryImpl implements SeriesEntry
{
	
	
	@Override
	public void clear()     
	{
		
	}

	@Override
	public int encode(EncodeIterator iter)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeInit(EncodeIterator iter, int maxEncodingSize)
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
	public Buffer encodedData() 
	{
		return null;
	}

	@Override
	public void encodedData(Buffer encodedData) 
	{
	   
	}
}
