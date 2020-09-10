package com.rtsdk.eta.valueadd.examples.common;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.valueadd.cache.CacheError;
import com.rtsdk.eta.valueadd.cache.CacheFactory;
import com.rtsdk.eta.valueadd.cache.PayloadCache;
import com.rtsdk.eta.valueadd.cache.PayloadCacheConfigOptions;
import com.rtsdk.eta.valueadd.cache.PayloadCursor;

/**
 * Cache information used by the UPA Value Add example applications.
 */
public class CacheInfo
{
	public boolean useCache = false;
	public PayloadCacheConfigOptions cacheOptions = null;
	public PayloadCache cache = null;
	public PayloadCursor cursor = null;
	public Buffer cacheDictionaryKey = null;
	public CacheError cacheError = null;
	
	public CacheInfo()
	{
		cacheOptions = CacheFactory.createPayloadCacheConfig();
		cacheDictionaryKey = CodecFactory.createBuffer();
		cacheError = CacheFactory.createCacheError();
	}
}
