package com.thomsonreuters.upa.valueadd.cache;


import java.io.PrintWriter;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.valueadd.cache.CacheError;
import com.thomsonreuters.upa.valueadd.cache.PayloadEntry;
import com.thomsonreuters.upa.valueadd.cache.PayloadCursor;



class PayloadEntryImpl implements PayloadEntry
{
	
	
	public static PayloadEntry create( PayloadCache cacheInstance, CacheError error)
	{
		return null;
	}
	
	@Override
	public void destroy()
	{
		
	}

	@Override
	public void clear()
	{
		
	}
	
	@Override
	public short dataType()
	{
		return DataTypes.UNKNOWN; 
		
	}

	@Override
	public int apply( DecodeIterator dIter,	Msg msg, CacheError error)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int retrieve( EncodeIterator eIter, PayloadCursor cursor, CacheError error)
	{

		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int trace( int traceFormat, PrintWriter fileWriter, DataDictionary dictionary )
	{
		return CodecReturnCodes.FAILURE;
	}

	

}