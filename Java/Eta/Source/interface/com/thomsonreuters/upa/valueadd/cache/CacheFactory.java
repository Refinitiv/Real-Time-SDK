package com.thomsonreuters.upa.valueadd.cache;

import com.thomsonreuters.upa.codec.CodecReturnCodes;

/**
 * Factory for creating UPA ValueAdd Cache package objects.
 */
public class CacheFactory
{
	private CacheFactory()
	{
		throw new AssertionError();
	}

	/**
	 * Creates an instance of a payload cache container.
	 *
	 * This is a thread safe method. 
	 * @param configOptions The options for configuring this cache container
	 * @param error Error information populated if this function fails
	 * @return The payload cache instance
	 * @see PayloadCache
	 * @see PayloadCacheConfigOptions
	 * @see CacheError
	 */
	public static PayloadCache createPayloadCache( PayloadCacheConfigOptions configOptions, CacheError error )
	{
		if ( error == null )
			throw new UnsupportedOperationException("CacheFactory.createPayloadCache: error cannot be null, PayloadCache not created.");
		else if ( configOptions == null )
		{
			error.errorId(CodecReturnCodes.FAILURE);
			error.text("CacheFactory.createPayloadCache error: received invalid cache config options");
			return null;
		}

		return PayloadCacheImpl.create(configOptions, error);
	}
	
	/**
	 * Creates an instance of a payload entry.
	 *
	 * This is a thread safe method.
	 * @param cacheInstance The cache where this entry will be created
	 * @param error Error information populated if the cache entry cannot be created
	 * @return The payload entry instance
	 * @see PayloadCache
	 * @see PayloadEntry
	 * @see CacheError
	 */
	public static PayloadEntry createPayloadEntry( PayloadCache cacheInstance, CacheError error )
	{
		if ( error == null )
			throw new UnsupportedOperationException("CacheFactory.createPayloadEntry: error cannot be null, PayloadEntry not created.");
		else if ( cacheInstance == null )
		{
			error.errorId(CodecReturnCodes.FAILURE);
			error.text("CacheFactory.createPayloadEntry error: received invalid cache instance");
			return null;
		}

		return PayloadEntryImpl.create(cacheInstance, error);
	}
	
	/**
	 * Creates an instance of a payload entry cursor.
	 *
	 * This is a thread safe method.
	 * @return The payload cursor
	 * @see PayloadCursor
	 */
	public static PayloadCursor createPayloadCursor()
	{
		return PayloadCursorImpl.create();
	}

	/**
	 * Creates an instance of a payload cache configuration options.
	 *
	 * This is a thread safe method.
	 * @return The payload cache configuration option instance
	 * @see PayloadCacheConfigOptions
	 */
	public static PayloadCacheConfigOptions createPayloadCacheConfig()
	{
		return new PayloadCacheConfigOptionsImpl();
	}
	
	/**
	 * Creates an instance of a cache error.
	 *
	 * This is a thread safe method.
	 * @return The cache error instance
	 * @see CacheError
	 */
	public static CacheError createCacheError()
	{
		return new CacheErrorImpl();
	}
}
