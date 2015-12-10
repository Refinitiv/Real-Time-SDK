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
import com.thomsonreuters.upa.examples.provider.ItemInfo;
import com.thomsonreuters.upa.examples.provider.ItemRequestInfo;
import com.thomsonreuters.upa.examples.provider.MarketByOrderItems;
import com.thomsonreuters.upa.examples.rdm.marketbyorder.MarketByOrderItem;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.valueadd.cache.PayloadEntry;
import com.thomsonreuters.upa.valueadd.examples.common.CacheHandler;
import com.thomsonreuters.upa.valueadd.examples.common.CacheInfo;

/**
 * Provides a method to encode msg from cached data.
 */
public class MarketByOrderCacheItems extends MarketByOrderItems
{
	/**
     * Encodes the market by order response. Returns success if encoding succeeds
     * or failure if encoding fails.
     * 
     * @param itemReqInfo - The item request related info
     * @param isSolicited - The response is solicited if set
     * @param msgBuf - The message buffer to encode the market price response into
     * @param serviceId - The service id of the market price response
     * @param dictionary - The dictionary used for encoding
     * @param error - error in case of encoding failure
     * @param cacheInfo - cache info to determine if cache data
     * @param cacheEntry - cache entry where to retrieve or apply data from/to cache
     * 
     * @return {@link CodecReturnCodes}
     */
    public int encodeResponse(ItemRequestInfo itemReqInfo, TransportBuffer msgBuf, boolean isSolicited,
    		int serviceId, DataDictionary dictionary, Error error, CacheInfo cacheInfo, PayloadEntry cacheEntry)
    {
    	ItemInfo itemInfo = itemReqInfo.itemInfo();
    	Channel channel = itemReqInfo.channel();
    	boolean isPrivateStream = itemReqInfo.isPrivateStreamRequest();
    	boolean isStreaming = itemReqInfo.isStreamingRequest();
    	int streamId = itemReqInfo.streamId();
    	Buffer itemName = itemInfo.itemName();
    	int ret = CodecReturnCodes.SUCCESS;
    	
        if (itemInfo.isRefreshRequired())
        {
            _marketByOrderRefresh.clear();
            _marketByOrderRefresh.dictionary(dictionary);

            // refresh complete
            _marketByOrderRefresh.applyRefreshComplete();

           
            // service Id
            _marketByOrderRefresh.serviceId(serviceId);
            _marketByOrderRefresh.applyHasServiceId();

            // QoS
            _marketByOrderRefresh.qos().dynamic(false);
            _marketByOrderRefresh.qos().timeliness(QosTimeliness.REALTIME);
            _marketByOrderRefresh.qos().rate(QosRates.TICK_BY_TICK);
            _marketByOrderRefresh.applyHasQos();

            // state
            if (isStreaming)
            {
                _marketByOrderRefresh.state().streamState(StreamStates.OPEN);
            }
            else
            {
                _marketByOrderRefresh.state().streamState(StreamStates.NON_STREAMING);
            }

            if (isPrivateStream)
            {
                _marketByOrderRefresh.applyPrivateStream();
            }
            _marketByOrderRefresh.state().dataState(DataStates.OK);
            _marketByOrderRefresh.state().code(StateCodes.NONE);
            _marketByOrderRefresh.state().text().data("Item Refresh Completed");

            if (isSolicited)
            {
                _marketByOrderRefresh.applySolicited();
                
                // clear cache for solicited refresh messages.
                _marketByOrderRefresh.applyClearCache();
            }
            
            _marketByOrderRefresh.streamId(streamId);
            _marketByOrderRefresh.itemName().data(itemName.data(), itemName.position(), itemName.length());
            _marketByOrderRefresh.marketByOrderItem((MarketByOrderItem)itemInfo.itemData());

            // encode
            _encodeIter.clear();
            ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
                return ret;
            }
            if ( cacheInfo.useCache && cacheEntry != null && cacheEntry.dataType() != DataTypes.UNKNOWN )
            {
            	
            	System.out.println("Encoding item " + itemReqInfo.itemName() + " from cache.");
            
            	 // set-up message
                Msg msg = _marketByOrderRefresh.encodeMsg();

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
	            ret = _marketByOrderRefresh.encode(_encodeIter);
	            if (ret != CodecReturnCodes.SUCCESS)
	            {
	                error.text("MarketByOrderRefresh.encode() failed");
	                return ret;
	            }
	            else if ( cacheInfo.useCache ) 
	            {
	            	System.out.println("Applying item " + itemReqInfo.itemName() + " to cache.");
	            	ret = CacheHandler.applyMsgBufferToCache(_encodeIter.majorVersion(), _encodeIter.minorVersion(), 
	            										cacheEntry, cacheInfo, msgBuf);
	            	if (ret != CodecReturnCodes.SUCCESS)
	            		error.text(" Error Applying payload to cache : " + cacheInfo.cacheError.text());
	            }
            }
        }
        else
        {
            _marketByOrderUpdate.clear();
            _marketByOrderUpdate.dictionary(dictionary);
            if (isPrivateStream)
            {
                _marketByOrderUpdate.applyPrivateStream();
            }
            _marketByOrderUpdate.streamId(streamId);
            _marketByOrderUpdate.marketByOrderItem((MarketByOrderItem)itemInfo.itemData());

            // encode
            _encodeIter.clear();
            ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
                return ret;
            }
            ret = _marketByOrderUpdate.encode(_encodeIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("MarketByOrderUpdate.encode() failed");
                return ret;
            }
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
}
