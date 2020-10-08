package com.refinitiv.eta.shared.provider;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderItem;
import com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderRefresh;
import com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderUpdate;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;

/**
 * This handles storage of all market by order items.
 * 
 * <p>
 * Provides methods generation of market by order data, for managing the list,
 * and a method for encoding a market by order message and payload.
 */
public class MarketByOrderItems
{
    private static final int MAX_MARKET_PRICE_ITEM_LIST_SIZE = 100;

    // item information list
    public List<MarketByOrderItem> _marketByOrderList;
    protected MarketByOrderRefresh _marketByOrderRefresh;
    protected MarketByOrderUpdate _marketByOrderUpdate;
    protected EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();

    /**
     * Instantiates a new market by order items.
     */
    public MarketByOrderItems()
    {
        _marketByOrderList = new ArrayList<MarketByOrderItem>(MAX_MARKET_PRICE_ITEM_LIST_SIZE);
        for (int i = 0; i < MAX_MARKET_PRICE_ITEM_LIST_SIZE; i++)
        {
            _marketByOrderList.add(new MarketByOrderItem());
        }
        _marketByOrderRefresh = new MarketByOrderRefresh();
        _marketByOrderUpdate = new MarketByOrderUpdate();
    }

    /**
     * Initializes market price item list.
     */
    public void init()
    {
        for (MarketByOrderItem mboItem : _marketByOrderList)
        {
            mboItem.clear();
        }
    }

    /**
     * Updates any item that's currently in use.
     */
    void update()
    {
        for (MarketByOrderItem mboItem : _marketByOrderList)
        {
            if (mboItem.isInUse)
                mboItem.updateFields();
        }
    }

    /**
     * Gets storage for a market by order item from the list.
     *
     * @param itemName the item name
     * @return the market by order item
     */
    public MarketByOrderItem get(String itemName)
    {
        // first try to find one with same name and reuse
        for (MarketByOrderItem mboItem : _marketByOrderList)
        {
            if (mboItem.isInUse && mboItem.itemName != null && mboItem.itemName.equals(itemName))
            {
                return mboItem;
            }
        }
        
        // next get a new one
        for (MarketByOrderItem mboItem : _marketByOrderList)
        {
            if (!mboItem.isInUse)
            {
                mboItem.initFields();
                mboItem.itemName = itemName;
                return mboItem;
            }
        }

        return null;
    }

    /**
     * Clears the item information.
     */
    public void clear()
    {
        for (MarketByOrderItem mboItem : _marketByOrderList)
        {
            mboItem.clear();
        }
    }

    /**
     * Encodes the market by order response. Returns success if encoding
     * succeeds or failure if encoding fails.
     *
     * @param channel - The channel to send a market by order response to
     * @param itemInfo - The item information
     * @param msgBuf - The message buffer to encode the market by order response
     *            into
     * @param isSolicited - The response is solicited if set
     * @param streamId - The stream id of the market by order response
     * @param isStreaming - Flag for streaming or snapshot
     * @param isPrivateStream the is private stream
     * @param serviceId - The service id of the market by order response
     * @param dictionary - The dictionary used for encoding
     * @param error - Error information in case of encoding failure
     * @return {@link CodecReturnCodes}
     */
    public int encodeResponse(Channel channel, ItemInfo itemInfo, TransportBuffer msgBuf, boolean isSolicited, int streamId, boolean isStreaming, boolean isPrivateStream, int serviceId, DataDictionary dictionary, Error error)
    {
        if (itemInfo.isRefreshRequired)
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
            _marketByOrderRefresh.itemName().data(itemInfo.itemName.data(), itemInfo.itemName.position(), itemInfo.itemName.length());
            _marketByOrderRefresh.marketByOrderItem((MarketByOrderItem)itemInfo.itemData);

            // encode
            _encodeIter.clear();
            int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
                return ret;
            }
            ret = _marketByOrderRefresh.encode(_encodeIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("MarketByOrderRefresh.encode() failed");
                return ret;
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
            _marketByOrderUpdate.marketByOrderItem((MarketByOrderItem)itemInfo.itemData);

            // encode
            _encodeIter.clear();
            int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
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
        }

        return CodecReturnCodes.SUCCESS;
    }
}
