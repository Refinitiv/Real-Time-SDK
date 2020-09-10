package com.rtsdk.eta.shared.provider;

import java.util.ArrayList;
import java.util.List;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataDictionary;
import com.rtsdk.eta.codec.DataStates;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Enum;
import com.rtsdk.eta.codec.QosRates;
import com.rtsdk.eta.codec.QosTimeliness;
import com.rtsdk.eta.codec.StateCodes;
import com.rtsdk.eta.codec.StreamStates;
import com.rtsdk.eta.shared.rdm.marketbyprice.MarketByPriceItem;
import com.rtsdk.eta.shared.rdm.marketbyprice.MarketByPriceRefresh;
import com.rtsdk.eta.shared.rdm.marketbyprice.MarketByPriceUpdate;
import com.rtsdk.eta.transport.Channel;
import com.rtsdk.eta.transport.Error;
import com.rtsdk.eta.transport.TransportBuffer;

/**
 * This handles storage of all market by price items.
 * 
 * <p>
 * Provides methods generation of market by price data, for managing the list, and a method for encoding a
 * market by price message and payload.
 */
public class MarketByPriceItems
{
    private static final int MAX_MARKET_PRICE_ITEM_LIST_SIZE = 100;

    // item information list
    private List<MarketByPriceItem> _marketByPriceList;

    private static Enum USD_ENUM;
    private static Enum BBO_ENUM;
    
    protected static final int MAX_ORDERS = 3; //Number of order in a single message
    
    protected MarketByPriceRefresh _marketByPriceRefresh;
    protected MarketByPriceUpdate _marketByPriceUpdate;

    protected EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();
    
    static
    {
        USD_ENUM = CodecFactory.createEnum();
        USD_ENUM.value(840);
        BBO_ENUM = CodecFactory.createEnum();
        BBO_ENUM.value(20);
    }


    /**
     * Instantiates a new market by price items.
     */
    public MarketByPriceItems()
    {
        _marketByPriceList = new ArrayList<MarketByPriceItem>(MAX_MARKET_PRICE_ITEM_LIST_SIZE);
        for (int i = 0; i < MAX_MARKET_PRICE_ITEM_LIST_SIZE; i++)
        {
            _marketByPriceList.add(new MarketByPriceItem());
        }

        _marketByPriceUpdate = new MarketByPriceUpdate();
        _marketByPriceRefresh = new MarketByPriceRefresh();
    }

    /**
     * Initializes market price item list.
     */
    public void init()
    {
        // clear item information list 
        for (MarketByPriceItem mbpItem : _marketByPriceList)
        {
            mbpItem.clear();
        }
    }

    /**
     * Updates any item that's currently in use.
     */
    void update()
    {
        for (MarketByPriceItem mbpItem : _marketByPriceList)
        {
            if(mbpItem.isInUse)
                mbpItem.updateFields();
        }
    }

    /**
     * Gets storage for a market by price item from the list.
     *
     * @param itemName the item name
     * @return the market by price item
     */
    public MarketByPriceItem get(String itemName)
    {
        // first try to find one with same name and reuse
        for (MarketByPriceItem mbpItem : _marketByPriceList)
        {
            if (mbpItem.isInUse && mbpItem.itemName != null && mbpItem.itemName.equals(itemName))
            {
                return mbpItem;
            }
        }
        
        // next get a new one
        for (MarketByPriceItem mbpItem : _marketByPriceList)
        {
            if (!mbpItem.isInUse)
            {
                mbpItem.initFields();
                mbpItem.itemName = itemName;
                return mbpItem;
            }
        }

        return null;
    }

    /**
     * Clears the item information.
     */
    public void clear()
    {
        for (MarketByPriceItem mbpItem : _marketByPriceList)
        {
            mbpItem.clear();
        }
    }

    /**
     * Encodes the market by price refresh. Returns success if encoding succeeds
     * or failure if encoding fails.
     *
     * @param channel - The channel to send a market by price refresh to
     * @param itemInfo - The item information
     * @param msgBuf - The message buffer to encode the market by price refresh
     *            into
     * @param isSolicited - The refresh is solicited if set
     * @param streamId - The stream id of the market by price refresh
     * @param isStreaming - Flag for streaming or snapshot
     * @param isPrivateStream the is private stream
     * @param serviceId - The service id of the market by price refresh
     * @param dictionary - The dictionary used for encoding
     * @param multiPartNo the multi part no
     * @param error the error
     * @return {@link CodecReturnCodes}
     */
    public int encodeRefresh(Channel channel, ItemInfo itemInfo, TransportBuffer msgBuf, boolean isSolicited, int streamId, boolean isStreaming, boolean isPrivateStream, int serviceId, DataDictionary dictionary, int multiPartNo, Error error)
    {
        _marketByPriceRefresh.clear();
        _marketByPriceRefresh.dictionary(dictionary);
        _marketByPriceRefresh.itemName().data(itemInfo.itemName.data(), itemInfo.itemName.position(), itemInfo.itemName.length());
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
        _marketByPriceRefresh.marketByPriceItem((MarketByPriceItem)itemInfo.itemData);
        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }
        ret = _marketByPriceRefresh.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
            error.text("MarketByPriceRefresh.encode() failed");
        
        return ret;
    }

    /**
     * Encodes the market by price update. Returns success if encoding succeeds
     * or failure if encoding fails.
     *
     * @param channel - The channel to send a market by price update to
     * @param itemInfo - The item information
     * @param msgBuf - The message buffer to encode the market by price update
     *            into
     * @param isSolicited - The update is solicited if set
     * @param streamId - The stream id of the market by price update
     * @param isStreamingRequest - Flag for streaming or snapshot
     * @param isPrivateStreamRequest - Flag for private stream request
     * @param serviceId - The service id of the market by price update
     * @param dictionary - The dictionary used for encoding
     * @param error - error in case of encoding failure
     * @return the int
     */
    public int encodeUpdate(Channel channel, ItemInfo itemInfo, TransportBuffer msgBuf, boolean isSolicited, int streamId, boolean isStreamingRequest, boolean isPrivateStreamRequest, int serviceId, DataDictionary dictionary, Error error)
    {
        _marketByPriceUpdate.clear();
        _marketByPriceUpdate.applyRefreshComplete();
        if(isPrivateStreamRequest)
            _marketByPriceUpdate.applyPrivateStream();
        
        _marketByPriceUpdate.dictionary(dictionary);
        _marketByPriceUpdate.itemName().data(itemInfo.itemName.data(), itemInfo.itemName.position(), itemInfo.itemName.length());
        _marketByPriceUpdate.streamId(streamId);
        _marketByPriceUpdate.dictionary(dictionary);
        _marketByPriceUpdate.marketByPriceItem((MarketByPriceItem)itemInfo.itemData);
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

        return ret;
    }
}