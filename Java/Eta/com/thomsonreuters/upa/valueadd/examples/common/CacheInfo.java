package com.thomsonreuters.upa.valueadd.examples.common;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.valueadd.cache.CacheError;
import com.thomsonreuters.upa.valueadd.cache.CacheFactory;
import com.thomsonreuters.upa.valueadd.cache.PayloadCache;
import com.thomsonreuters.upa.valueadd.cache.PayloadCacheConfigOptions;
import com.thomsonreuters.upa.valueadd.cache.PayloadCursor;

/**
 * Cache information used by the UPA Value Add example applications.
 */
public class CacheInfo
{
	public boolean useCache = false;
	public PayloadCacheConfigOptions cacheOptions = null;;
	public PayloadCache cache = null;;
	public PayloadCursor cursor = null;;
	public Buffer cacheDictionaryKey = null;
	public CacheError cacheError = null;
	
	public CacheInfo()
	{
		cacheOptions = CacheFactory.createPayloadCacheConfig();
		cacheDictionaryKey = CodecFactory.createBuffer();
		cacheError = CacheFactory.createCacheError();
	}
}
