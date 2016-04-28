package com.thomsonreuters.upa.valueadd.cache;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.valueadd.cache.PayloadCacheConfigOptions;

/*
 * Cache configuration class
 */
class PayloadCacheConfigOptionsImpl implements PayloadCacheConfigOptions
{
	
	@Override
	public int maxItems() 
	{
		return CodecReturnCodes.FAILURE;
	}
	
	@Override
	public void maxItems(int maxItems) 
	{
		
	}
	
	public void clear() 
	{
		
	}
}

