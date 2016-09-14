package com.thomsonreuters.upa.perftools.common;

import java.util.HashMap;
import java.util.Map;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;

/** Represents one channel's session. Stores information about items requested. */
public class ProviderSession
{
    // client sessions over this limit gets rejected with NAK mount
    public static final int NUM_CLIENT_SESSIONS = 5;

    private ItemWatchlist _refreshItemList;                     // list of items to send refreshes for
    private ItemWatchlist _updateItemList;                      // list of items to send updates for
    private ItemWatchlist _genMsgItemList;                      // list of items to send generic messages for
    private Map<ItemAttributes, ItemInfo> _itemAttributesTable; // Open items indexed by attributes
    private Map<Integer, ItemInfo> _itemStreamIdTable;          // Open items indexed by stream id
    private int _openItemCount;                                 // Count of items currently open
    private ClientChannelInfo _clientChannelInfo;               // Client channel information.
    private TransportBuffer _writingBuffer;                     // Current buffer in use by this channel.
    private int _packedBufferCount;                             // Total number of buffers currently packed in _writingBuffer
    private long _timeActivated;                                // Time at which this channel was fully setup.
    private ItemEncoder _itemEncoder;                           // item encoder
    private XmlMsgData _xmlMsgData;                             // Msgs from XML
    private int _unexpectedCloseCount;                          // Count of unexpected close messages received
    private ProviderThread _providerThread;                     // Provider thread of this session.

    public ProviderSession(XmlMsgData xmlMsgData, ItemEncoder itemEncoder)
    {
        _refreshItemList = new ItemWatchlist(100000);
        _updateItemList = new ItemWatchlist(100000);
        _genMsgItemList = new ItemWatchlist(100000);
        _itemAttributesTable = new HashMap<ItemAttributes, ItemInfo>(100000);
        _itemStreamIdTable = new HashMap<Integer, ItemInfo>(100000);
        _xmlMsgData = xmlMsgData;
        _itemEncoder = itemEncoder;
    }

    /**
     * Initializes for one client channel.
     */
    public void init(ClientChannelInfo clientChannelInfo)
    {
        _refreshItemList.init();
        _updateItemList.init();
        _genMsgItemList.init();
        _openItemCount = 0;
        _packedBufferCount = 0;
        _timeActivated = 0;
        _clientChannelInfo = clientChannelInfo;
        _unexpectedCloseCount = 0;
    }

    /**
     * Clears provider session object.
     */
    public void cleanup()
    {
        _refreshItemList.clear();
        _updateItemList.clear();
        _genMsgItemList.clear();
        _openItemCount = 0;
        _packedBufferCount = 0;
        _timeActivated = 0;
        _clientChannelInfo = null;
        _unexpectedCloseCount = 0;
    }

    /**
     * Creates item information to be published by provider.
     * 
     * @param attributes - item attributes
     * @param streamId - stream id
     * @return - Created item information.
     */
    public ItemInfo createItemInfo(ItemAttributes attributes, int streamId)
    {
        ItemInfo itemInfo = new ItemInfo();
        itemInfo.attributes(attributes);
        itemInfo.streamId(streamId);

        switch (attributes.domainType())
        {
            case DomainTypes.MARKET_PRICE:
                if (_xmlMsgData.marketPriceUpdateMsgCount() == 0)
                {
                    System.err.println("createItemInfo: No MarketPrice data present in message data file.");
                    return null;
                }
                itemInfo.itemData(new MarketPriceItem());
                break;
            default:
                System.err.println("Unsupported domain " + attributes.domainType());
                return null;
        }

        // Add item to watchlist
        _refreshItemList.add(itemInfo);

        _openItemCount++;

        return itemInfo;

    }

    /**
     * Clears item information.
     */
    public void freeItemInfo(ItemInfo itemInfo)
    {
        if (itemInfo == null)
            return;

        switch (itemInfo.attributes().domainType())
        {
            case DomainTypes.MARKET_PRICE:
                MarketPriceItem mpItem = (MarketPriceItem)itemInfo.itemData();
                mpItem.iMsg(0);
                break;
        }
        itemAttributesTable().remove(itemInfo.attributes());
        itemStreamIdTable().remove(itemInfo.streamId());
        if (itemInfo.attributes().msgKey().checkHasName())
        {
            itemInfo.attributes().msgKey().name().clear();
        }
        if (itemInfo.attributes().msgKey().checkHasAttrib())
        {
            itemInfo.attributes().msgKey().encodedAttrib().clear();
        }
        itemInfo.attributes().msgKey().clear();
        itemInfo.clear();
        --_openItemCount;
    }

    /**
     * Prints estimated size of messages being published.
     * 
     * @param error - Error information populated when there is a failure to
     *            obtain buffers for measuring size.
     * 
     * @return <0 if failure, 0 otherwise.
     */
    public int printEstimatedMsgSizes(Error error)
    {
        MsgKey msgKey = CodecFactory.createMsgKey();
        msgKey.flags(MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_SERVICE_ID);
        msgKey.nameType(InstrumentNameTypes.RIC);
        msgKey.name().data("RDT0");
        msgKey.serviceId(0);

        if (_xmlMsgData.marketPriceUpdateMsgCount() > 0)
        {
            MarketPriceItem mpItem = new MarketPriceItem();
            ItemInfo itemInfo = new ItemInfo();
            itemInfo.attributes().msgKey(msgKey);
            itemInfo.attributes().domainType(DomainTypes.MARKET_PRICE);
            itemInfo.itemData(mpItem);

            int bufLen = _itemEncoder.estimateRefreshBufferLength(itemInfo);
            TransportBuffer testBuffer = _clientChannelInfo.channel.getBuffer(bufLen, false, error);
            if (testBuffer == null)
                return PerfToolsReturnCodes.FAILURE;
            int ret = _itemEncoder.encodeRefresh(_clientChannelInfo.channel, itemInfo, testBuffer, null, 0, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return PerfToolsReturnCodes.FAILURE;
            }
            System.out.println("Approximate message size:");
            System.out.printf("  MarketPrice RefreshMsg (without name): \n");
            System.out.printf("         estimated length: %d bytes\n", bufLen);
            System.out.printf("    approx encoded length: %d bytes\n", testBuffer.length());
            _clientChannelInfo.channel.releaseBuffer(testBuffer, error);

            // Update msgs
            for (int i = 0; i < _xmlMsgData.marketPriceUpdateMsgCount(); ++i)
            {
                bufLen = _itemEncoder.estimateUpdateBufferLength(itemInfo);
                testBuffer = _clientChannelInfo.channel.getBuffer(bufLen, false, error);
                if (testBuffer == null)
                    return PerfToolsReturnCodes.FAILURE;
                ret = _itemEncoder.encodeUpdate(_clientChannelInfo.channel, itemInfo, testBuffer, null, 0, error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return PerfToolsReturnCodes.FAILURE;
                }
                System.out.printf("  MarketPrice UpdateMsg %d: \n", i + 1);
                System.out.printf("         estimated length: %d bytes\n", bufLen);
                System.out.printf("    approx encoded length: %d bytes\n", testBuffer.length());
                _clientChannelInfo.channel.releaseBuffer(testBuffer, error);

            }
        }

        System.out.println();

        return PerfToolsReturnCodes.SUCCESS;
    }

    /**
     * Handles item stream close.
     * 
     * @param streamId
     */
    public void closeItemStream(int streamId)
    {
        ItemInfo itemInfo = (ItemInfo)_itemStreamIdTable.get(streamId);
        if (itemInfo != null)
        {
            itemInfo.clear();
        }
        else
        {
            /*
             * If we are sending an update for an item to the platform while it
             * is sending us a close for that same item, it may respond to our
             * update with another close. If so, this is okay, so don't close
             * the channel because of it.
             */
        	if (_unexpectedCloseCount == 0)
        	{
                System.out.printf("Received unexpected close on stream %d(this may just be an extra close from the platform). \n", streamId);
        	}
            _unexpectedCloseCount++;
        }
    }

    /**
     * Returns item lists to send refresh messages to. 
     */
    public ItemWatchlist refreshItemList()
    {
        return _refreshItemList;
    }

    /**
     * Returns item lists to send update messages to. 
     */
    public ItemWatchlist updateItemList()
    {
        return _updateItemList;
    }

    /**
     * Returns item lists to send generic messages to. 
     */
    public ItemWatchlist genMsgItemList()
    {
        return _genMsgItemList;
    }

    /**
     * Returns open items indexed by attributes.
     */
    public Map<ItemAttributes, ItemInfo> itemAttributesTable()
    {
        return _itemAttributesTable;
    }

    /**
     * Returns open items indexed by stream id.
     */
    public Map<Integer, ItemInfo> itemStreamIdTable()
    {
        return _itemStreamIdTable;
    }

    /**
     * Returns number of items currently opened.
     */
    public int openItemCount()
    {
        return _openItemCount;
    }

    /**
     * Returns number of unexpected close messages received.
     */
    public int unexpectedCloseCount()
    {
        return _unexpectedCloseCount;
    }

    /**
     * Client channel information.
     */
    public ClientChannelInfo clientChannelInfo()
    {
        return _clientChannelInfo;
    }

    /**
     * Current buffer in use by this client channel.
     */
    public TransportBuffer writingBuffer()
    {
        return _writingBuffer;
    }

    /**
     * Sets current buffer to use by this client channel.
     */
    public void writingBuffer(TransportBuffer writingBuffer)
    {
        _writingBuffer = writingBuffer;
    }

    /**
     * Total number of buffers currently packed in _writingBuffer.
     */
    public int packedBufferCount()
    {
        return _packedBufferCount;
    }

    /**
     * Total number of buffers currently packed in _writingBuffer.
     */
    public void packedBufferCount(int packedBufferCount)
    {
        _packedBufferCount = packedBufferCount;
    }

    /**
     * Time in nano seconds at which this channel was fully setup.
     */
    public long timeActivated()
    {
        return _timeActivated;
    }

    /**
     * Time in nano seconds at which this channel was fully setup.
     */
    public void timeActivated(long timeActivated)
    {
        _timeActivated = timeActivated;
    }
    
    /**
     * Provider thread of this session.
     */
    public ProviderThread providerThread()
    {
        return _providerThread;
    }

    /**
     * Provider thread of this session.
     */
    public void providerThread(ProviderThread providerThread)
    {
        _providerThread = providerThread;
    }
}
