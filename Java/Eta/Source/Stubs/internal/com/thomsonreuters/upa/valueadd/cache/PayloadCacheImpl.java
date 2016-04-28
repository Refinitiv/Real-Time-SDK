package com.thomsonreuters.upa.valueadd.cache;


import java.util.List;

import com.thomsonreuters.upa.valueadd.cache.CacheError;
import com.thomsonreuters.upa.valueadd.cache.PayloadCache;
import com.thomsonreuters.upa.valueadd.cache.PayloadEntry;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;





class PayloadCacheImpl implements PayloadCache
{
	
	public static PayloadCache create( PayloadCacheConfigOptions configOptions, CacheError error )
	{
		return null;
	}
	
	@Override
	public void destroy()
	{
		
	}
	
	@Override
	public void destroyAll()
	{
	
	}

	@Override
	public int setDictionary( DataDictionary fidDictionary, String dictionaryKey, CacheError error )
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int setSharedDictionaryKey( String dictionaryKey, CacheError error )
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int entryCount()
	{
		return CodecReturnCodes.FAILURE;
	}
	
	@Override
	public List<PayloadEntry> entryList()
	{
		return null;
	}

	@Override
	public void clear()
	{
		
	}

	
}