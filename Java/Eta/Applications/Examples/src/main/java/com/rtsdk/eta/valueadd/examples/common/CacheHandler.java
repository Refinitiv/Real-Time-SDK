package com.rtsdk.eta.valueadd.examples.common;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.transport.TransportBuffer;
import com.rtsdk.eta.valueadd.cache.CacheFactory;
import com.rtsdk.eta.valueadd.cache.PayloadEntry;

/**
 * This is the cache handler for the UPA Value Add example applications.
 */
public class CacheHandler
{
	static public PayloadEntry createCacheEntry(CacheInfo cacheInfo)
	{
		PayloadEntry cacheEntry = null;
		cacheEntry = CacheFactory.createPayloadEntry(cacheInfo.cache, cacheInfo.cacheError);
		if (cacheEntry == null)
			System.out.println("Failed to create cache entry. Error (" + cacheInfo.cacheError.errorId() + 
					") : " + cacheInfo.cacheError.text());
			
		return cacheEntry;
	}
	
	static public int applyMsgBufferToCache(int majorVersion, int minorVersion, PayloadEntry cacheEntry, CacheInfo cacheInfo, TransportBuffer buffer)
	{
		if ( cacheEntry ==  null )
		{
			cacheInfo.cacheError.errorId(CodecReturnCodes.INVALID_ARGUMENT);
			cacheInfo.cacheError.text("Invalid cache entry");
			return CodecReturnCodes.FAILURE;
		}

		int ret = CodecReturnCodes.FAILURE;
		DecodeIterator dIter = CodecFactory.createDecodeIterator();
		dIter.clear();
		if ( (ret = dIter.setBufferAndRWFVersion(buffer, majorVersion, majorVersion)) != CodecReturnCodes.SUCCESS )
		{
			System.out.println("Failed to set iterator on data buffer: Error (" + ret + ")");
			return ret;
		}

		Msg msg = CodecFactory.createMsg();
		msg.clear();
		if ((ret = msg.decode(dIter)) != CodecReturnCodes.SUCCESS)
		{
			System.out.println("Failed to decode message: Error (" + ret + ")");
			return ret;
		}

		if ((ret = cacheEntry.apply(dIter, msg, cacheInfo.cacheError)) != CodecReturnCodes.SUCCESS)
		{
			System.out.println("Failed to apply payload data to cache. Error (" + cacheInfo.cacheError.errorId() + 
						") : " + cacheInfo.cacheError.text());

			return ret;
		}

		return CodecReturnCodes.SUCCESS;
	}
	
	static public int applyMsgToCache(DecodeIterator dIter, PayloadEntry cacheEntry, CacheInfo cacheInfo, Msg msg)
	{
		if ( cacheEntry ==  null )
		{
			cacheInfo.cacheError.errorId(CodecReturnCodes.INVALID_ARGUMENT);
			cacheInfo.cacheError.text("Invalid cache entry");
			return CodecReturnCodes.FAILURE;
		}

		int ret = CodecReturnCodes.FAILURE;
		if ((ret = cacheEntry.apply(dIter, msg, cacheInfo.cacheError)) != CodecReturnCodes.SUCCESS)
		{
			System.out.println("Failed to apply payload data to cache. Error (" + cacheInfo.cacheError.errorId() + 
					") : " + cacheInfo.cacheError.text());

			return ret;
		}


		return CodecReturnCodes.SUCCESS;
	}
	
	static public int retrieveFromCache(EncodeIterator eIter, PayloadEntry cacheEntry, CacheInfo cacheInfo)
	{
		if (cacheEntry != null)
		{
			// retrieve payload from cache
			return cacheEntry.retrieve(eIter, cacheInfo.cursor, cacheInfo.cacheError);
		}
		else
		{
			System.out.println("Error: no cache entry object\n");
			return CodecReturnCodes.FAILURE;
		}
	}
	
}
