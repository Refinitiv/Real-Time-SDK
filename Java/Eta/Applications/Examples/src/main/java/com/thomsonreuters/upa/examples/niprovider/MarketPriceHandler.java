package com.thomsonreuters.upa.examples.niprovider;

import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.niprovider.StreamIdWatchList.WatchListEntry;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceClose;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceRefresh;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceResponseBase;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceUpdate;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;

/**
 * This is the market price handler for the UPA NIProvider application. It
 * provides methods to encode and send refreshes and updates, as well as close
 * streams.
 */
public class MarketPriceHandler
{
    public static int TRANSPORT_BUFFER_SIZE_MESSAGE = ChannelSession.MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

    private int domainType;

    private MarketPriceRefresh marketPriceRefresh;
    private MarketPriceUpdate marketPriceUpdate;
    private MarketPriceClose closeMessage;

    private final StreamIdWatchList watchList; // stream states based on
                                               // response

    // reusable variables used for encoding
    protected FieldList fieldList = CodecFactory.createFieldList();
    protected FieldEntry fieldEntry = CodecFactory.createFieldEntry();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();

    /**
     * Instantiates a new market price handler.
     *
     * @param watchList the watch list
     * @param dictionary the dictionary
     */
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
    
    private int closeStream(ChannelSession chnl, int streamId, Error error)
    {
        //get a buffer for the item close
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        //encode item close
        closeMessage.clear();
        closeMessage.streamId(streamId);
        closeMessage.domainType(domainType);
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());

        int ret = closeMessage.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("encodeMarketPriceClose(): Failed <"
                        + CodecReturnCodes.toString(ret) + ">");
        }
        return chnl.write(msgBuf, error);
    }

    /**
     * Encodes and sends item refreshes for MarketPrice domain.
     * 
     * @param chnl The channel to send a refresh to.
     * 
     * @param itemNames List of item names.
     * 
     * @param serviceInfo RDM directory response information.
     * 
     * @param error Populated if an error occurs.
     * 
     * @return success if item refreshes can be made, can be encoded and sent
     *         successfully. Failure if encoding/sending refresh failed.
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
        marketPriceRefresh.applyRefreshComplete();

        //service Id
        marketPriceRefresh.serviceId(serviceInfo.serviceId());
        marketPriceRefresh.applyHasServiceId();
        marketPriceUpdate.serviceId(serviceInfo.serviceId());
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
    private int sendRefreshes(ChannelSession chnl, List<String> itemNames, Error error)
    {
        int ret = CodecReturnCodes.SUCCESS;
        for (String itemName : itemNames)
        {
            Integer streamId = watchList.add(domainType, itemName);

            marketPriceRefresh.itemName().data(itemName);
            marketPriceRefresh.streamId(streamId);
            marketPriceRefresh.marketPriceItem(watchList.get(streamId).marketPriceItem);

            ret = encodeAndSendContent(chnl, marketPriceRefresh, error);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Encodes and sends item updates for MarketPrice domain.
     * 
     * @param chnl The channel to send a refresh to.
     * @param error Populated if an error occurs.
     * @return success if item refreshes can be made, can be encoded and sent
     *         successfully. Failure if encoding/sending refresh failed.
     */
    public int sendItemUpdates(ChannelSession chnl, Error error)
    {
        int ret = CodecReturnCodes.SUCCESS;
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

            ret = encodeAndSendContent(chnl, marketPriceUpdate, error);
            if (ret < CodecReturnCodes.SUCCESS)
                return ret;
        }

        return ret;
    }
    
    private int encodeAndSendContent(ChannelSession chnl,
            MarketPriceResponseBase marketPriceContent, Error error)
    {
        //get a buffer for the item refresh/update
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_MESSAGE, false,
                                                         error);

        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());

        int ret = marketPriceContent.encode(encIter);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            error.text("MarketPriceResponse.encode failed");
            error.errorId(ret);
            return ret;
        }

        return chnl.write(msgBuf, error);
    }
    
    /**
     * Close all item streams.
     *
     * @param chnl The channel to send a item stream close to.
     * @param error the error
     * @return the int
     */
    public int closeStreams(ChannelSession chnl, Error error)
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

            closeStream(chnl, entry.getKey().intValue(), error);
            iter.remove();
        }

        return CodecReturnCodes.SUCCESS;
    }

}
