package com.thomsonreuters.upa.valueadd.examples.provider;


import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.valueadd.examples.common.MarketPriceItem;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.valueadd.cache.PayloadEntry;
import com.thomsonreuters.upa.valueadd.examples.common.CacheHandler;
import com.thomsonreuters.upa.valueadd.examples.common.CacheInfo;

/**
 * Provides a method to encode msg from cached data.
 */
public class MarketPriceCacheItems extends MarketPriceItems
{
    /**
     * Encodes the market price response. Returns success if encoding succeeds
     * or failure if encoding fails.
     * 
     * @param itemReqInfo - The item request related info
     * @param isSolicited - The response is solicited if set
     * @param msgBuf - The message buffer to encode the market price response
     *            into
     * @param serviceId - The service id of the market price response
     * @param dictionary - The dictionary used for encoding
     * @param error - error in case of encoding failure
     * @param cacheInfo - cache info to determine if cache data
     * @param cacheEntry - cache entry where to retrieve or apply data from/to cache
     * 
     * @return {@link CodecReturnCodes}
     */
    public int encodeResponse(ItemRequestInfo itemReqInfo, TransportBuffer msgBuf, boolean isSolicited, int serviceId,
    							DataDictionary dictionary, Error error, CacheInfo cacheInfo, PayloadEntry cacheEntry)
    
    {
    	ItemInfo itemInfo = itemReqInfo.itemInfo();
    	Channel channel = itemReqInfo.channel();
    	boolean isPrivateStream = itemReqInfo.isPrivateStreamRequest();
    	boolean isStreaming = itemReqInfo.isStreamingRequest();
    	int streamId = itemReqInfo.streamId();
    	Buffer itemName = itemInfo.itemName();
    	
        MarketPriceItem mpItem = (MarketPriceItem)itemInfo.itemData();

        // set message depending on whether refresh or update
        if (itemInfo.isRefreshRequired())
        {
            _marketPriceRefresh.clear();
            _marketPriceRefresh.dictionary(dictionary);
            if (isStreaming)
            {
                _marketPriceRefresh.state().streamState(StreamStates.OPEN);
            }
            else
            {
                _marketPriceRefresh.state().streamState(StreamStates.NON_STREAMING);
            }
            _marketPriceRefresh.state().dataState(DataStates.OK);
            _marketPriceRefresh.state().code(StateCodes.NONE);
            _marketPriceRefresh.state().text().data("Item Refresh Completed");
            _marketPriceRefresh.applyRefreshComplete();
            _marketPriceRefresh.applyHasQos();

            if (isSolicited)
            {
                _marketPriceRefresh.applySolicited();
                
                // clear cache for solicited refresh messages.
                _marketPriceRefresh.applyClearCache();
            }

            if (isPrivateStream)
            {
                _marketPriceRefresh.applyPrivateStream();
            }

            // Service Id
            _marketPriceRefresh.applyHasServiceId();
            _marketPriceRefresh.serviceId(serviceId);

            // ItemName
            _marketPriceRefresh.itemName().data(itemName.data(), itemName.position(), itemName.length());

            // Qos
            _marketPriceRefresh.qos().dynamic(false);
            _marketPriceRefresh.qos().rate(QosRates.TICK_BY_TICK);
            _marketPriceRefresh.qos().timeliness(QosTimeliness.REALTIME);

            // StreamId
            _marketPriceRefresh.streamId(streamId);
            _marketPriceRefresh.marketPriceItem(mpItem);

            // encode
            _encodeIter.clear();
            int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
                return ret;
            }
            
            if ( cacheInfo.useCache && cacheEntry != null && cacheEntry.dataType() != DataTypes.UNKNOWN )
            {
            	
            	System.out.println("Encoding item " + itemReqInfo.itemName() + " from cache.");
            
            	 // set-up message
                Msg msg = _marketPriceRefresh.encodeMsg();

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
            	
            	ret = msg.encodeComplete(_encodeIter, true);
            	if (ret < CodecReturnCodes.SUCCESS)
            		return ret;
            }
            else
            {
	            ret = _marketPriceRefresh.encode(_encodeIter);
	            if (ret != CodecReturnCodes.SUCCESS)
	                error.text("MarketPriceRefresh.encode() failed");
	            else if ( cacheInfo.useCache ) 
	            {
	            	System.out.println("Applying item " + itemReqInfo.itemName() + " to cache.");
	            	ret = CacheHandler.applyMsgBufferToCache(_encodeIter.majorVersion(), _encodeIter.minorVersion(), 
	            										cacheEntry, cacheInfo, msgBuf);
	            	if (ret != CodecReturnCodes.SUCCESS)
	            		error.text(" Error Applying payload to cache : " + cacheInfo.cacheError.text());
	            }
            }
            
            return ret;
        }
        else
        {
            _marketPriceUpdate.clear();
            _marketPriceUpdate.dictionary(dictionary);

            // this is an update message
            _marketPriceUpdate.streamId(streamId);

            // include msg key in updates for non-interactive provider streams
            if (streamId < 0)
            {
                _marketPriceUpdate.applyHasServiceId();

                // ServiceId
                _marketPriceUpdate.serviceId(serviceId);

                // Itemname
                _marketPriceUpdate.itemName().data(itemName.data(), itemName.position(), itemName.length());
            }

            if (isPrivateStream)
            {
                _marketPriceUpdate.applyPrivateStream();
            }

            _marketPriceUpdate.marketPriceItem(mpItem);
            
            // encode
            _encodeIter.clear();
            int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
                return ret;
            }
            ret = _marketPriceUpdate.encode(_encodeIter);
            if (ret != CodecReturnCodes.SUCCESS)
                error.text("MarketPriceUpdate.encode() failed");
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
}
