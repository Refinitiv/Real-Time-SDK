/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.common;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.valueadd.cache.CacheError;
import com.refinitiv.eta.valueadd.cache.CacheFactory;
import com.refinitiv.eta.valueadd.cache.PayloadCache;
import com.refinitiv.eta.valueadd.cache.PayloadCacheConfigOptions;
import com.refinitiv.eta.valueadd.cache.PayloadCursor;

/**
 * Cache information used by the ETA Value Add example applications.
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
