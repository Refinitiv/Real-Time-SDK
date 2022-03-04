/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.niprovider;

import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FieldEntry;
import com.refinitiv.eta.codec.FieldList;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.valueadd.examples.common.CacheHandler;
import com.refinitiv.eta.valueadd.examples.common.CacheInfo;
import com.refinitiv.eta.valueadd.examples.niprovider.StreamIdWatchList.WatchListEntry;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel.State;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceClose;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceRefresh;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceResponseBase;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceUpdate;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.TransportBuffer;

/*
 * This is the market price handler for the ETA NIProvider application. It
 * provides methods to encode and send refreshes and updates, as well as close
 * streams.
 */
class MarketPriceHandler
{
    public static int TRANSPORT_BUFFER_SIZE_MESSAGE = 1024;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = 1024;

    private int domainType;

    private MarketPriceRefresh marketPriceRefresh;
    private MarketPriceUpdate marketPriceUpdate;
    private MarketPriceClose closeMessage;

    private final StreamIdWatchList watchList; // stream states based on response

    // reusable variables used for encoding
    protected FieldList fieldList = CodecFactory.createFieldList();
    protected FieldEntry fieldEntry = CodecFactory.createFieldEntry();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();

    private ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
    
    public MarketPriceHandler(StreamIdWatchList watchList, DataDictionary dictionary)
    {
        this(watchList, DomainTypes.MARKET_PRICE, dictionary);
    }

    protected MarketPriceHandler(StreamIdWatchList watchList, int domainType,
            DataDictionary dictionary)
    {

        this.watchList = watchList;
        this.domainType = domainType;
        marketPriceRefresh = createMarketPriceRefresh();
        marketPriceRefresh.dictionary(dictionary);
        marketPriceUpdate = createMarketPriceUpdate();
        marketPriceUpdate.dictionary(dictionary);
        closeMessage = new MarketPriceClose();
    }

    protected MarketPriceRefresh createMarketPriceRefresh()
    {
        return new MarketPriceRefresh();
    }

    protected MarketPriceUpdate createMarketPriceUpdate()
    {
        return new MarketPriceUpdate();
    }
    
    private int closeStream(ReactorChannel chnl, int streamId, ReactorErrorInfo errorInfo)
    {
    	if(chnl.state() == State.UP)
    	{
        //get a buffer for the item close
        TransportBuffer msgBuf = chnl.getBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, errorInfo);
        if (msgBuf == null)
            return ReactorReturnCodes.FAILURE;

        //encode item close
        closeMessage.clear();
        closeMessage.streamId(streamId);
        closeMessage.domainType(domainType);
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());

        int ret = closeMessage.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("encodeMarketPriceClose(): Failed <"
                        + CodecReturnCodes.toString(ret) + ">");
        }
        return chnl.submit(msgBuf, submitOptions, errorInfo);
    }

    		
    	return ReactorReturnCodes.SUCCESS;
    }

    /*
     * Encodes and sends item refreshes for MarketPrice domain.
     */
    int sendItemRefreshes(ReactorChannel chnl, List<String> itemNames, int serviceId, CacheInfo cacheInfo,
            ReactorErrorInfo errorInfo)
    {
        if (itemNames == null || itemNames.isEmpty())
            return CodecReturnCodes.SUCCESS;

        generateRefreshAndUpdate(serviceId);

        return sendRefreshes(chnl, itemNames, cacheInfo, errorInfo);
    }
    
    private void generateRefreshAndUpdate(int serviceId)
    {
        //refresh complete
        marketPriceRefresh.applyRefreshComplete();

        //service Id
        marketPriceRefresh.serviceId(serviceId);
        marketPriceRefresh.applyHasServiceId();
        marketPriceUpdate.serviceId(serviceId);
        marketPriceUpdate.applyHasServiceId();

        //QoS
        marketPriceRefresh.qos().dynamic(false);
        marketPriceRefresh.qos().timeliness(QosTimeliness.REALTIME);
        marketPriceRefresh.qos().rate(QosRates.TICK_BY_TICK);
        marketPriceRefresh.applyHasQos();

        //state
        marketPriceRefresh.state().streamState(StreamStates.OPEN);
        marketPriceRefresh.state().dataState(DataStates.OK);
        marketPriceRefresh.state().code(StateCodes.NONE);
        marketPriceRefresh.state().text().data("Item Refresh Completed");
    }

    //sends one item at a time
    private int sendRefreshes(ReactorChannel chnl, List<String> itemNames, CacheInfo cacheInfo, ReactorErrorInfo errorInfo)
    {
        int ret = CodecReturnCodes.SUCCESS;
        for (String itemName : itemNames)
        {
            Integer streamId = watchList.add(domainType, itemName);

            marketPriceRefresh.itemName().data(itemName);
            marketPriceRefresh.streamId(streamId);
            marketPriceRefresh.marketPriceItem(watchList.get(streamId).marketPriceItem);

            ret = encodeAndSendContent(chnl, marketPriceRefresh, cacheInfo, watchList.get(streamId), true, errorInfo);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /*
     * Encodes and sends item updates for MarketPrice domain.
     */
    int sendItemUpdates(ReactorChannel chnl, CacheInfo cacheInfo, ReactorErrorInfo errorInfo)
    {
        int ret = CodecReturnCodes.SUCCESS;
    	int itemCount = 0;
        for (Map.Entry<Integer, WatchListEntry> mapEntry : watchList)
        {
            WatchListEntry wle = mapEntry.getValue();
            if (mapEntry.getValue().domainType != domainType)
            {
                //this entry is from a different domainType, skip
                continue;
            }
            //update fields
            wle.marketPriceItem.updateFields();

            marketPriceUpdate.streamId(mapEntry.getKey().intValue());
            marketPriceUpdate.itemName().data(wle.itemName);
            marketPriceUpdate.marketPriceItem(wle.marketPriceItem);

            ret = encodeAndSendContent(chnl, marketPriceUpdate, cacheInfo, wle, false, errorInfo);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
            itemCount++;
        }

        if (itemCount > 0)
        	System.out.println("Sent " + itemCount + " MarketPrice items.");
        return ret;
    }
    
    private int encodeAndSendContent(ReactorChannel chnl, MarketPriceResponseBase marketPriceContent,
    		CacheInfo cacheInfo, WatchListEntry wle, boolean isRefresh, ReactorErrorInfo errorInfo)
    {
        //get a buffer for the item refresh/update
        TransportBuffer msgBuf = chnl.getBuffer(TRANSPORT_BUFFER_SIZE_MESSAGE, false, errorInfo);

        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());

        if ( isRefresh && cacheInfo.useCache && wle.cacheEntry != null )
        {
        	Msg msg = marketPriceContent.encodeMsg();
            int ret = msg.encodeInit(encIter, 0);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;

            ret = CacheHandler.retrieveFromCache(encIter, wle.cacheEntry, cacheInfo);
        	if (ret != CodecReturnCodes.SUCCESS)
        	{
        		errorInfo.error().text(" Error retrieving payload from cache : " + cacheInfo.cacheError.text());
        		return ret;
        	}
        	
            ret = msg.encodeComplete(encIter, true);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
        }
        else
        {
	        int ret = marketPriceContent.encode(encIter);
	        if (ret < CodecReturnCodes.SUCCESS)
	        {
	            errorInfo.error().text("MarketPriceResponse.encode failed");
	            errorInfo.error().errorId(ret);
	            return ret;
	        }
	        else if (cacheInfo.useCache) 
	        {
	        	System.out.println("Applying item " + wle.itemName + " to cache.");
	        	if (wle.cacheEntry == null)
	        		wle.cacheEntry = CacheHandler.createCacheEntry(cacheInfo);
	        	ret = CacheHandler.applyMsgBufferToCache(encIter.majorVersion(), encIter.minorVersion(), 
	        										wle.cacheEntry, cacheInfo, msgBuf);
	        	if (ret != CodecReturnCodes.SUCCESS)
	        		 errorInfo.error().text(" Error Applying payload to cache : " + cacheInfo.cacheError.text());
	        }
        }
        return chnl.submit(msgBuf, submitOptions, errorInfo);
    }
    
    /*
     * Close all item streams.
     */
    int closeStreams(ReactorChannel chnl, ReactorErrorInfo errorInfo)
    {
        Iterator<Entry<Integer, WatchListEntry>> iter = watchList.iterator();
        while (iter.hasNext())
        {
            Map.Entry<Integer, WatchListEntry> entry = iter.next();

            if (entry.getValue().domainType != domainType)
            {
                //this entry is from a different domainType, skip
                continue;
            }

            closeStream(chnl, entry.getKey().intValue(), errorInfo);
            iter.remove();
        }

        return CodecReturnCodes.SUCCESS;
    }

}


 