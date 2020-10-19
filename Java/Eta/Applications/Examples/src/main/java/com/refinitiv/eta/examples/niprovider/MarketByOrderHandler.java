package com.refinitiv.eta.examples.niprovider;

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
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.examples.common.ChannelSession;
import com.refinitiv.eta.examples.niprovider.StreamIdWatchList.WatchListEntry;
import com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderClose;
import com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderRefresh;
import com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderResponseBase;
import com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderUpdate;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;

/**
 * This is the market by order handler for the ETA NIProvider application. It
 * provides methods to encode and send refreshes and updates, as well as
 * closing streams.
 */
public class MarketByOrderHandler
{
    public static int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

    private int domainType;

    private MarketByOrderRefresh marketByOrderRefresh;
    private MarketByOrderUpdate marketByOrderUpdate;
    private MarketByOrderClose closeMessage;

    private final StreamIdWatchList watchList; // stream states based on
                                               // response

    // reusable variables used for encoding
    protected FieldList fieldList = CodecFactory.createFieldList();
    protected FieldEntry fieldEntry = CodecFactory.createFieldEntry();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();

    /**
     * Instantiates a new market by order handler.
     *
     * @param watchList the watch list
     * @param dictionary the dictionary
     */
    public MarketByOrderHandler(StreamIdWatchList watchList, DataDictionary dictionary)
    {
        this(watchList, DomainTypes.MARKET_BY_ORDER, dictionary);
    }

    protected MarketByOrderHandler(StreamIdWatchList watchList, int domainType, DataDictionary dictionary)
    {
        this.watchList = watchList;
        this.domainType = domainType;
        marketByOrderRefresh = createMarketByOrderRefresh();
        marketByOrderRefresh.dictionary(dictionary);
        marketByOrderUpdate = createMarketByOrderUpdate();
        marketByOrderUpdate.dictionary(dictionary);
        closeMessage = new MarketByOrderClose();
    }

    protected MarketByOrderRefresh createMarketByOrderRefresh()
    {
        return new MarketByOrderRefresh();
    }
    
    protected MarketByOrderUpdate createMarketByOrderUpdate()
    {
        return new MarketByOrderUpdate();
    }
    
    private int closeStream(ChannelSession chnl, int streamId, Error error)
    {
        //get a buffer for the item close
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        //encode item close
        closeMessage.clear();
        closeMessage.streamId(streamId);
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());

        int ret = closeMessage.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("encodeMarketByOrderClose(): Failed <"
                        + CodecReturnCodes.toString(ret) + ">");
        }
        return chnl.write(msgBuf, error);
    }

    /**
     * Close all item streams.
     *
     * @param chnl The channel to send a item stream close to.
     * @param error Populated if an error occurs.
     * @return the int
     */
    public int closeStreams(ChannelSession chnl, Error error)
    {
        Iterator<Entry<Integer, WatchListEntry>> iter = watchList.iterator();
        while(iter.hasNext())
        {
            Map.Entry<Integer, WatchListEntry> entry = iter.next();
         
            if (entry.getValue().domainType != domainType)
            {
                /* this entry is from a different domainType, skip */
                continue;
            }

            closeStream(chnl, entry.getKey().intValue(), error);
            iter.remove();
        }

        watchList.clear();

        return CodecReturnCodes.SUCCESS;
    }
    
    /**
     * Encodes and sends item refreshes for market by order domain.
     *
     * @param chnl The channel to send a refresh to.
     * @param itemNames List of item names.
     * @param serviceInfo the service info
     * @param error Populated if an error occurs.
     * @return success if item refreshes can be made, can be encoded and sent
     *         successfully. Failure if encoding/sending refreshes failed.
     */
    public int sendItemRefreshes(ChannelSession chnl, List<String> itemNames, Service serviceInfo,
            Error error)
    {
        if (itemNames == null || itemNames.isEmpty())
            return CodecReturnCodes.SUCCESS;

        generateRefreshAndUpdate(serviceInfo);

        return sendRefreshes(chnl, itemNames, error);
    }

    private void generateRefreshAndUpdate(Service serviceInfo)
    {
        //refresh complete
        marketByOrderRefresh.applyRefreshComplete();

        //service Id
        marketByOrderRefresh.serviceId(serviceInfo.serviceId());
        marketByOrderRefresh.applyHasServiceId();
        marketByOrderUpdate.serviceId(serviceInfo.serviceId());
        marketByOrderUpdate.applyHasServiceId();

        //QoS
        marketByOrderRefresh.qos().dynamic(false);
        marketByOrderRefresh.qos().timeliness(QosTimeliness.REALTIME);
        marketByOrderRefresh.qos().rate(QosRates.TICK_BY_TICK);
        marketByOrderRefresh.applyHasQos();

        //state
        marketByOrderRefresh.state().streamState(StreamStates.OPEN);
        marketByOrderRefresh.state().dataState(DataStates.OK);
        marketByOrderRefresh.state().code(StateCodes.NONE);
        marketByOrderRefresh.state().text().data("Item Refresh Completed");
    }

    private int sendRefreshes(ChannelSession chnl, List<String> itemNames, Error error)
    {
        int ret = CodecReturnCodes.SUCCESS;
        for (String itemName : itemNames)
        {
            Integer streamId = watchList.add(domainType, itemName);

            marketByOrderRefresh.itemName().data(itemName);
            marketByOrderRefresh.streamId(streamId);
            marketByOrderRefresh
                    .marketByOrderItem(watchList.get(streamId).marketByOrderItem);

            ret = encodeAndSendContent(chnl, marketByOrderRefresh, error);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Encodes and sends item updates for market by order domain.
     * 
     * @param chnl The channel to send a refresh to.
     * @param error Populated if an error occurs.
     * 
     * @return success if item updates can be made, can be encoded and sent
     *         successfully. Failure if encoding/sending updates failed.
     */
    public int sendItemUpdates(ChannelSession chnl, Error error)
    {
        int ret = CodecReturnCodes.SUCCESS;
        for (Entry<Integer, WatchListEntry> mapEntry : watchList)
        {
            WatchListEntry wle = mapEntry.getValue();
            if (mapEntry.getValue().domainType != domainType)
            {
                /* this entry is from a different domainType, skip */
                continue;
            }
            /* update fields */
            wle.marketByOrderItem.updateFields();

            marketByOrderUpdate.streamId(mapEntry.getKey().intValue());
            marketByOrderUpdate.itemName().data(wle.itemName);
            marketByOrderUpdate.marketByOrderItem(wle.marketByOrderItem);

            ret = encodeAndSendContent(chnl, marketByOrderUpdate, error);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
        }

        return ret;
    }

    private int encodeAndSendContent(ChannelSession chnl, MarketByOrderResponseBase marketContent,
            Error error)
    {
        //get a buffer for the item request
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false,
                                                         error);

        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());

        int ret = marketContent.encode(encIter);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            error.text("MarketByOrderResponse.encode failed");
            error.errorId(ret);
            return ret;
        }

        return chnl.write(msgBuf, error);
    }

}
