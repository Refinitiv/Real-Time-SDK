package com.refinitiv.eta.valueadd.cache;

/**
 * Configuration options used when creating a {@link PayloadCache} instance.
 *
 * @see CacheFactory
 */
public interface PayloadCacheConfigOptions
{
	/**
     * Limit on the number of payload entries in this cache. Zero indicates no limit.
     * 
     * @return the max limit
     */
	public int maxItems();
	
	/**
	 * Limit on the number of payload entries in this cache. Zero indicates no limit.
	 * 
	 * @param maxItems the max limit to set
	 */
	public void maxItems(int maxItems); 
}

