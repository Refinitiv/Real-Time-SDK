/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.provider;


import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.shared.provider.ItemInfo;
import com.refinitiv.eta.shared.provider.ItemRequestInfo;
import com.refinitiv.eta.shared.provider.MarketByPriceItems;
import com.refinitiv.eta.shared.rdm.marketbyprice.MarketByPriceItem;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.cache.PayloadEntry;
import com.refinitiv.eta.valueadd.examples.common.CacheHandler;
import com.refinitiv.eta.valueadd.examples.common.CacheInfo;

/**
 * Provides a method to encode msg from cached data.
 */
public class MarketByPriceCacheItems extends MarketByPriceItems
{
    
    /**
     * Encodes the market by price refresh. Returns success if encoding succeeds
     * or failure if encoding fails.
     *
     * @param itemReqInfo - The item request related info
     * @param msgBuf - The message buffer to encode the market price response
     *            into
     * @param isSolicited - The response is solicited if set
     * @param serviceId - The service id of the market price response
     * @param dictionary - The dictionary used for encoding
     * @param multiPartNo the multi part no
     * @param error - error in case of encoding failure
     * @param cacheInfo - cache info to determine if cache data
     * @param cacheEntry - cache entry where to retrieve or apply data from/to cache
     * @return {@link CodecReturnCodes}
     */
    public int encodeRefresh(ItemRequestInfo itemReqInfo, TransportBuffer msgBuf, boolean isSolicited, int serviceId,
    		DataDictionary dictionary, int multiPartNo, Error error, CacheInfo cacheInfo, PayloadEntry cacheEntry)
    {
    	ItemInfo itemInfo = itemReqInfo.itemInfo();
    	Channel channel = itemReqInfo.channel();
    	boolean isPrivateStream = itemReqInfo.isPrivateStreamRequest();
    	boolean isStreaming = itemReqInfo.isStreamingRequest();
    	int streamId = itemReqInfo.streamId();
    	Buffer itemName = itemInfo.itemName();
    	int ret = CodecReturnCodes.SUCCESS;
    	
        _marketByPriceRefresh.clear();
        _marketByPriceRefresh.dictionary(dictionary);
        _marketByPriceRefresh.itemName().data(itemName.data(), itemName.position(), itemName.length());
        _marketByPriceRefresh.streamId(streamId);
        _marketByPriceRefresh.applyHasServiceId();
        _marketByPriceRefresh.serviceId(serviceId);
        if (isSolicited)
        {
            _marketByPriceRefresh.applySolicited();
            
            // set clear cache on first part of all solicted refreshes.
            if(multiPartNo == 0)
            {
                _marketByPriceRefresh.applyClearCache();
            }
        }

        if (isPrivateStream)
            _marketByPriceRefresh.applyPrivateStream();

        
        _marketByPriceRefresh.dictionary(dictionary);
        
        // QoS 
        _marketByPriceRefresh.qos().dynamic(false);
        _marketByPriceRefresh.qos().timeliness(QosTimeliness.REALTIME);
        _marketByPriceRefresh.qos().rate(QosRates.TICK_BY_TICK);
        _marketByPriceRefresh.applyHasQos();

        // State 
        _marketByPriceRefresh.state().streamState(isStreaming ? StreamStates.OPEN : StreamStates.NON_STREAMING);
        _marketByPriceRefresh.state().dataState(DataStates.OK);
        _marketByPriceRefresh.state().code(StateCodes.NONE);
        
        _marketByPriceRefresh.marketByPriceItem((MarketByPriceItem)itemInfo.itemData());
        
       	// multi-part refresh complete when multiPartNo hits max
        if (multiPartNo == MAX_ORDERS - 1)
        {
            _marketByPriceRefresh.applyRefreshComplete();
            _marketByPriceRefresh.state().text().data("Item Refresh Completed");
        }
        else
        {
            _marketByPriceRefresh.state().text().data("Item Refresh In Progress");
        }
        _marketByPriceRefresh.partNo(multiPartNo);
        
        _encodeIter.clear();
        ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }
        ret = _marketByPriceRefresh.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
            error.text("MarketByPriceRefresh.encode() failed");
        else if ( cacheInfo.useCache ) 
        {
        	System.out.println("Applying item " + itemReqInfo.itemName() + " to cache.");
        	ret = CacheHandler.applyMsgBufferToCache(_encodeIter.majorVersion(), _encodeIter.minorVersion(), 
        										cacheEntry, cacheInfo, msgBuf);
        	if (ret != CodecReturnCodes.SUCCESS)
        		error.text(" Error Applying payload to cache : " + cacheInfo.cacheError.text());
        }
        
        return ret;
    }
    
    /**
     * Encode refresh from cache.
     *
     * @param itemReqInfo the item req info
     * @param msgBuf the msg buf
     * @param isSolicited the is solicited
     * @param serviceId the service id
     * @param dictionary the dictionary
     * @param multiPartNo the multi part no
     * @param error the error
     * @param cacheInfo the cache info
     * @param cacheEntry the cache entry
     * @return the int
     */
    public int encodeRefreshFromCache(ItemRequestInfo itemReqInfo, TransportBuffer msgBuf, boolean isSolicited, int serviceId,
    		DataDictionary dictionary, int multiPartNo, Error error, CacheInfo cacheInfo, PayloadEntry cacheEntry)
    {
    	ItemInfo itemInfo = itemReqInfo.itemInfo();
    	Channel channel = itemReqInfo.channel();
    	boolean isPrivateStream = itemReqInfo.isPrivateStreamRequest();
    	boolean isStreaming = itemReqInfo.isStreamingRequest();
    	int streamId = itemReqInfo.streamId();
    	Buffer itemName = itemInfo.itemName();
    	int ret = CodecReturnCodes.SUCCESS;
    	
    	_marketByPriceRefresh.clear();
        _marketByPriceRefresh.dictionary(dictionary);
        _marketByPriceRefresh.itemName().data(itemName.data(), itemName.position(), itemName.length());
        _marketByPriceRefresh.streamId(streamId);
        _marketByPriceRefresh.applyHasServiceId();
        _marketByPriceRefresh.serviceId(serviceId);
        if (isSolicited)
        {
            _marketByPriceRefresh.applySolicited();
            
            // set clear cache on first part of all solicted refreshes.
            if(multiPartNo == 0)
            {
                _marketByPriceRefresh.applyClearCache();
            }
        }

        if (isPrivateStream)
            _marketByPriceRefresh.applyPrivateStream();

        
        _marketByPriceRefresh.dictionary(dictionary);
        
        // QoS 
        _marketByPriceRefresh.qos().dynamic(false);
        _marketByPriceRefresh.qos().timeliness(QosTimeliness.REALTIME);
        _marketByPriceRefresh.qos().rate(QosRates.TICK_BY_TICK);
        _marketByPriceRefresh.applyHasQos();

        // State 
        _marketByPriceRefresh.state().streamState(isStreaming ? StreamStates.OPEN : StreamStates.NON_STREAMING);
        _marketByPriceRefresh.state().dataState(DataStates.OK);
        _marketByPriceRefresh.state().code(StateCodes.NONE);
        _marketByPriceRefresh.state().text().data("Item Refresh");
        
        _marketByPriceRefresh.marketByPriceItem((MarketByPriceItem)itemInfo.itemData());
         
        _marketByPriceRefresh.partNo(multiPartNo);
        
        _encodeIter.clear();
        ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }	
        
        // set-up message
        Msg msg = _marketByPriceRefresh.encodeMsg();

        // encode message
        ret = msg.encodeInit(_encodeIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;
   
    	ret = CacheHandler.retrieveFromCache(_encodeIter, cacheEntry, cacheInfo);
    	if (ret != CodecReturnCodes.SUCCESS)
    	{
    		error.text(" Error retrieving payload from cache : " + cacheInfo.cacheError.text());
    		return ret;
    	}
    	
    	if (cacheInfo.cursor.isComplete())
    		_encodeIter.setRefreshCompleteFlag();
    	
    	ret = msg.encodeComplete(_encodeIter, true);
    	if (ret < CodecReturnCodes.SUCCESS)
    		return ret;
    	
       	return ret;
    }

    /**
     * Encodes the market by price update. Returns success if encoding succeeds
     * or failure if encoding fails.
     *
     * @param itemReqInfo - The item request related info
     * @param msgBuf - The message buffer to encode the market price response into
     * @param isSolicited - The response is solicited if set
     * @param serviceId - The service id of the market price response
     * @param dictionary - The dictionary used for encoding
     * @param error - error in case of encoding failure
     * @param cacheInfo - cache info to determine if cache data
     * @param cacheEntry - cache entry where to retrieve or apply data from/to cache
     * @return {@link CodecReturnCodes}
     */
    public int encodeUpdate(ItemRequestInfo itemReqInfo, TransportBuffer msgBuf, boolean isSolicited,
    					int serviceId, DataDictionary dictionary, Error error, CacheInfo cacheInfo, PayloadEntry cacheEntry)
    {
    	ItemInfo itemInfo = itemReqInfo.itemInfo();
    	Channel channel = itemReqInfo.channel();
    	boolean isPrivateStream = itemReqInfo.isPrivateStreamRequest();
    	int streamId = itemReqInfo.streamId();
    	Buffer itemName = itemInfo.itemName();
    	
    	
        _marketByPriceUpdate.clear();
        _marketByPriceUpdate.applyRefreshComplete();
        if(isPrivateStream)
            _marketByPriceUpdate.applyPrivateStream();
        
        _marketByPriceUpdate.dictionary(dictionary);
        _marketByPriceUpdate.itemName().data(itemName.data(), itemName.position(), itemName.length());
        _marketByPriceUpdate.streamId(streamId);
        _marketByPriceUpdate.dictionary(dictionary);
        _marketByPriceUpdate.marketByPriceItem((MarketByPriceItem)itemInfo.itemData());
        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }
        ret = _marketByPriceUpdate.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
            error.text("MarketByPriceUpdate.encode() failed");
        else if ( cacheInfo.useCache ) 
        {
        	System.out.println("Applying item " + itemReqInfo.itemName() + " to cache.");
        	ret = CacheHandler.applyMsgBufferToCache(_encodeIter.majorVersion(), _encodeIter.minorVersion(), 
        										cacheEntry, cacheInfo, msgBuf);
        	if (ret != CodecReturnCodes.SUCCESS)
        		error.text(" Error Applying payload to cache : " + cacheInfo.cacheError.text());
        }

        return ret;
    }
}