/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.provider;

import java.nio.ByteBuffer;
import java.util.EnumSet;

import com.refinitiv.eta.codec.AckMsg;
import com.refinitiv.eta.codec.AckMsgFlags;
import com.refinitiv.eta.codec.Array;
import com.refinitiv.eta.codec.ArrayEntry;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.MsgKeyFlags;
import com.refinitiv.eta.codec.NakCodes;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RefreshMsgFlags;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.RequestMsgFlags;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StatusMsgFlags;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.codec.UpdateMsgFlags;
import com.refinitiv.eta.shared.LoginRequestInfo;
import com.refinitiv.eta.shared.ProviderDirectoryHandler;
import com.refinitiv.eta.shared.ProviderSession;
import com.refinitiv.eta.shared.provider.ItemInfo;
import com.refinitiv.eta.shared.provider.ItemInfoList;
import com.refinitiv.eta.shared.provider.ItemRejectReason;
import com.refinitiv.eta.shared.provider.ItemRequestInfo;
import com.refinitiv.eta.shared.provider.ItemRequestInfoList;
import com.refinitiv.eta.shared.provider.MarketByOrderItems;
import com.refinitiv.eta.shared.provider.MarketByPriceItems;
import com.refinitiv.eta.shared.provider.MarketPriceItems;
import com.refinitiv.eta.shared.provider.SymbolListItems;
import com.refinitiv.eta.shared.rdm.marketbyprice.MarketByPriceItem;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceItem;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceStatus;
import com.refinitiv.eta.examples.common.ProviderDictionaryHandler;
import com.refinitiv.eta.examples.common.ProviderLoginHandler;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;

/**
 * This is the implementation of handling item requests for the interactive
 * provider application.
 * <p>
 * It provides methods for processing item requests from consumers and sending
 * back the refresh/update messages. Methods for sending request reject/close
 * status messages, initializing the item handler, checking if the item count
 * per channel has been reached, checking if an item is already opened on a
 * channel, checking if a stream is already in use, and closing item streams are
 * also provided.
 * <p>
 * This handler provides data for MarketPrice, MarketByPrice and MarketByOrder
 * item requests.
 */
public class ItemHandler
{
    private static final int REJECT_MSG_SIZE = 1024;
    private static final int ACK_MSG_SIZE = 1024;
    private static final int ITEM_MSG_SIZE = 1024;
    private static final int MAX_REFRESH_PARTS = 3;
    private static final int POST_MSG_SIZE = 1024;
    private static final Buffer _triItemName = CodecFactory.createBuffer();
    private static final Buffer _privateStreamItemName = CodecFactory.createBuffer();
    private static final Buffer _slNameBuf = CodecFactory.createBuffer();
    private static final Buffer _batchReqName = CodecFactory.createBuffer();

    private MarketPriceStatus _marketPriceStatus;
    private ProviderSession _providerSession;
    private SymbolListItems _symbolListItemWatchList;
    private MarketByOrderItems _marketByOrderItemWatchList;
    private MarketByPriceItems _marketByPriceItemWatchList;
    private MarketPriceItems _marketPriceItemWatchList;
    private ItemInfoList _itemInfoWatchList;
    private ItemRequestInfoList _itemRequestWatchList;

    private ProviderDictionaryHandler _dictionaryHandler;
    private ProviderLoginHandler _loginHandler;

    private int _serviceId;
    private int _itemCount = 0;

    private Error _error = TransportFactory.createError();
    private ElementList _elementList = CodecFactory.createElementList();
    private ElementEntry _elementEntry = CodecFactory.createElementEntry();
    private Array _array = CodecFactory.createArray();
    private ArrayEntry _arrayEntry = CodecFactory.createArrayEntry();
    private EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();
    private AckMsg _ackMsg = (AckMsg)CodecFactory.createMsg();
    private Msg _nestedMsg = CodecFactory.createMsg();
    private UpdateMsg _updateMsg = (UpdateMsg)CodecFactory.createMsg();
    private StatusMsg _statusMsg = (StatusMsg)CodecFactory.createMsg();
    private Qos _providerQos = CodecFactory.createQos();

    /**
     * Instantiates a new item handler.
     *
     * @param providerSession the provider session
     * @param dictionaryHandler the dictionary handler
     * @param loginHandler the login handler
     */
    public ItemHandler(ProviderSession providerSession, ProviderDictionaryHandler dictionaryHandler, ProviderLoginHandler loginHandler)
    {
        _providerSession = providerSession;
        _marketByOrderItemWatchList = new MarketByOrderItems();
        _marketByPriceItemWatchList = new MarketByPriceItems();
        _marketPriceItemWatchList = new MarketPriceItems();
        _marketPriceStatus = new MarketPriceStatus();
        _symbolListItemWatchList = new SymbolListItems();
        _itemInfoWatchList = new ItemInfoList();
        _itemRequestWatchList = new ItemRequestInfoList();

        _dictionaryHandler = dictionaryHandler;
        _loginHandler = loginHandler;

        _updateMsg.msgClass(MsgClasses.UPDATE);
        _ackMsg.msgClass(MsgClasses.ACK);
        _statusMsg.msgClass(MsgClasses.STATUS);

        _triItemName.data("TRI");
        _privateStreamItemName.data("RES-DS");
        _slNameBuf.data("_ETA_ITEM_LIST");
        _batchReqName.data(":ItemList");
        
        //set Qos for provider
        _providerQos.dynamic(false);
        _providerQos.rate(QosRates.TICK_BY_TICK);
        _providerQos.timeliness(QosTimeliness.REALTIME);
    }

    /**
     * Initializes item handler internal structures.
     */
    public void init()
    {
        _itemRequestWatchList.init();
        _itemInfoWatchList.init();
        _marketByOrderItemWatchList.init();
        _marketByPriceItemWatchList.init();
        _marketPriceItemWatchList.init();
        _symbolListItemWatchList.init();
    }

    /**
     * Processes an item request. This consists of storing the request
     * information, then calling sendItemResponse() to send the response.
     * 
     * @param chnl - The channel of the response
     * @param msg - The partially decoded message
     * @param dIter - The decode iterator
     * @param error the error
     * @return returns success if decoding of request message and sending of
     *         response message succeeds or failure if it fails.
     */
    public int processRequest(Channel chnl, Msg msg, DecodeIterator dIter, Error error)
    {
        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
                if (!msg.msgKey().checkHasServiceId() || msg.msgKey().serviceId() != serviceId())
                {
                    return sendItemRequestReject(chnl, msg.streamId(), msg.domainType(), ItemRejectReason.INVALID_SERVICE_ID, false, error);
                }

                //check if QoS supported 
                if ((msg.flags() & RequestMsgFlags.HAS_WORST_QOS) != 0 &&
                        (msg.flags() & RequestMsgFlags.HAS_QOS) != 0)
                {
                    if(!_providerQos.isInRange(((RequestMsg)msg).qos(), ((RequestMsg)msg).worstQos()))
                    {
                        return sendItemRequestReject(chnl, msg.streamId(),
                                                msg.domainType(), ItemRejectReason.QOS_NOT_SUPPORTED, false, error);
                    }
                }
                else if ((msg.flags() & RequestMsgFlags.HAS_QOS) != 0)
                {
                    if (!((RequestMsg)msg).qos().equals(_providerQos))
                    {
                        return sendItemRequestReject(chnl, msg.streamId(),
                                                msg.domainType(), ItemRejectReason.QOS_NOT_SUPPORTED, false, error);
                    }
                }
                
                //check for unsupported key attribute information
                if (msg.msgKey().checkHasAttrib())
                {
                    return sendItemRequestReject(chnl, msg.streamId(), msg.domainType(), ItemRejectReason.KEY_ENC_ATTRIB_NOT_SUPPORTED, false, error);
                }

                if (((RequestMsg)msg).checkHasBatch())
                {
                    return processBatchRequest(chnl, msg, dIter, ((RequestMsg)msg).checkPrivateStream(), error);
                }
                else
                {
                    return processSingleItemRequest(chnl, msg, dIter, ((RequestMsg)msg).checkPrivateStream(), error);
                }
            case MsgClasses.CLOSE:
                System.out.println("Received Item close for streamId " + msg.streamId());
                closeStream(chnl, msg.streamId());
                break;
            case MsgClasses.POST:
                return processPost(chnl, msg, dIter, error);
            default:
                error.text("Received Unhandled Item Msg Class: " + MsgClasses.toString(msg.msgClass()));
                return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private int processBatchRequest(Channel chnl, Msg msg, DecodeIterator dIter, boolean isPrivateStream, Error error)
    {
        System.out.println("Received batch item request (streamId=" + msg.streamId() + ") on domain " + DomainTypes.toString(msg.domainType()));

        /* check if batch stream already in use with a different key */
        if (isStreamInUse(chnl, msg.streamId(), msg.msgKey()))
        {
            return sendItemRequestReject(chnl, msg.streamId(), msg.domainType(), ItemRejectReason.STREAM_ALREADY_IN_USE, isPrivateStream, error);
        }

        // The payload of a batch request contains an elementList
        int ret = _elementList.decode(dIter, null);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            return sendItemRequestReject(chnl, msg.streamId(), msg.domainType(), ItemRejectReason.REQUEST_DECODE_ERROR, isPrivateStream, error);
        }

        EnumSet<ItemRejectReason> rejectReasonSet = EnumSet.noneOf(ItemRejectReason.class);
        int dataState = DataStates.NO_CHANGE;
        // The list of items being requested is in an elementList entry with the
        // element name of ":ItemList"
        while ((ret = _elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (_elementEntry.name().equals(_batchReqName))
            {
                // The list of items names is in an array
                ret = _array.decode(dIter);
                if (ret < CodecReturnCodes.SUCCESS)
                {
                    error.text("Array.decode() for batch request failed with return code: " + CodecReturnCodes.toString(ret));
                    break;
                }

                // Get each requested item name
                // We will assign consecutive stream IDs for each item
                // starting with the stream following the one the batch request
                // was made on
                int itemStream = msg.streamId();

                while ((ret = _arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                {
                    if (ret < CodecReturnCodes.SUCCESS)
                    {
                        error.text("ArrayEntry.decode() failed with return code: " + CodecReturnCodes.toString(ret));
                        dataState = DataStates.SUSPECT;
                        continue;
                    }

                    //check if stream already in use with a different key
                    itemStream++; // increment stream ID with each item we find
                                  // in the batch request

                    
                    //check for private stream special item name without
                    //private stream flag set
                    if (!isPrivateStream && _arrayEntry.encodedData().equals(_privateStreamItemName))
                    {
                        sendItemRequestReject(chnl, itemStream, msg.domainType(), ItemRejectReason.PRIVATE_STREAM_REDIRECT, isPrivateStream, error);
                        dataState = DataStates.SUSPECT;
                        continue;
                    }

                    //all of the items requested have the same key. They use
                    //the key of the batch request.
                    //The only difference is the name
                    msg.msgKey().flags(msg.msgKey().flags() | MsgKeyFlags.HAS_NAME);
                    msg.msgKey().name(_arrayEntry.encodedData());

                    ItemRequestInfo itemReqInfo = getMatchingItemReqInfo(chnl, msg, itemStream, rejectReasonSet);
                    ItemRejectReason rejectReason = rejectReasonSet.iterator().hasNext() ? rejectReasonSet.iterator().next() : ItemRejectReason.NONE;
                    if (itemReqInfo == null && rejectReason != ItemRejectReason.NONE)
                    {
                        sendItemRequestReject(chnl, itemStream, msg.domainType(), rejectReason, isPrivateStream, error);
                        dataState = DataStates.SUSPECT;
                        continue;
                    }
                    else if (itemReqInfo != null)
                    {
                        // Batch requests should not be used to reissue item
                        // requests.
                        sendItemRequestReject(chnl, itemStream, msg.domainType(), ItemRejectReason.BATCH_ITEM_REISSUE, isPrivateStream, error);
                        dataState = DataStates.SUSPECT;
                        continue;
                    }

                    rejectReasonSet.clear();
                    itemReqInfo = getNewItemReqInfo(chnl, msg, itemStream, rejectReasonSet);
                    if (itemReqInfo == null)
                    {
                        // Batch requests should not be used to reissue item
                        // requests.
                        sendItemRequestReject(chnl, itemStream, msg.domainType(), rejectReasonSet.iterator().next(), isPrivateStream, error);
                        dataState = DataStates.SUSPECT;
                        continue;
                    }

                    if (((RequestMsg)msg).checkPrivateStream())
                    {
                        System.out.println("Received Private Stream Item Request for " + itemReqInfo.itemName + "(streamId=" + itemStream + ")  on domain " + DomainTypes.toString(itemReqInfo.domainType));
                    }
                    else
                    {
                        System.out.println("Received Item Request for " + itemReqInfo.itemName + "(streamId=" + itemStream + ") on domain " + DomainTypes.toString(itemReqInfo.domainType));
                    }

                    //send item response/refresh if required
                    if (!((RequestMsg)msg).checkNoRefresh())
                        sendItemResponse(itemReqInfo, error);

                    if (!itemReqInfo.isStreamingRequest) 
                    {
                        //snapshot request - so we dont have to send updates
                        //free item request info
                        freeItemReqInfo(itemReqInfo);
                    }
                    else
                    {
                        itemReqInfo.itemInfo.isRefreshRequired = false;
                    }
                }
            }
        }
        //now that we have processed the batch request and sent responses for
        //all the items, send a response for the batch request itself
        //get a buffer for the batch status close
        TransportBuffer msgBuf = chnl.getBuffer(ACK_MSG_SIZE, false, error);
        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        //we close the stream the batch request was made on (and later send the
        //item responses on different streams)
        ret = encodeBatchCloseStatus(chnl, msg.domainType(), msgBuf, msg.streamId(), dataState, error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        //send batch status close
        return _providerSession.write(chnl, msgBuf, error);
    }

    private int encodeBatchCloseStatus(Channel chnl, int domainType, TransportBuffer msgBuf, int streamId, int dataState, Error error)
    {
        _statusMsg.clear();

        /* set-up message */
        _statusMsg.msgClass(MsgClasses.STATUS);
        _statusMsg.streamId(streamId);
        _statusMsg.domainType(domainType);
        _statusMsg.containerType(DataTypes.NO_DATA);
        _statusMsg.flags(StatusMsgFlags.HAS_STATE);
        _statusMsg.state().streamState(StreamStates.CLOSED);
        _statusMsg.state().dataState(dataState);
        _statusMsg.state().code(StateCodes.NONE);
        _statusMsg.state().text().data("Stream closed for batch");

        /* clear encode iterator */
        _encodeIter.clear();

        /* encode message */
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }
        ret = _statusMsg.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("StatusMsg.encode() failed");
            return ret;
        }
        return CodecReturnCodes.SUCCESS;
    }

    private int processSingleItemRequest(Channel chnl, Msg msg, DecodeIterator dIter, boolean isPrivateStream, Error error)
    {
        int domainType = msg.domainType();

        //check for private stream special item name without private stream
        //flag set
        if (!isPrivateStream && _privateStreamItemName.equals(msg.msgKey().name()))
        {
            return sendItemRequestReject(chnl, msg.streamId(), domainType, ItemRejectReason.PRIVATE_STREAM_REDIRECT, isPrivateStream, error);
        }

        //check for invalid symbol list request
        if ((domainType == DomainTypes.SYMBOL_LIST) && (msg.msgKey().name() != null))
        {
            //if the consumer specified symbol list name isn't
            //"_ETA_ITEM_LIST", reject it
            if (!msg.msgKey().name().equals(_slNameBuf))
            {
                return sendItemRequestReject(chnl, msg.streamId(), domainType, ItemRejectReason.ITEM_NOT_SUPPORTED, isPrivateStream, error);
            }
        }

        //get request info structure
        //check if item already opened with exact same key on another stream 
        EnumSet<ItemRejectReason> rejectReasonSet = EnumSet.noneOf(ItemRejectReason.class);
        
        //Check for reissue request
        ItemRequestInfo itemReqInfo = getMatchingItemReqInfo(chnl, msg, msg.streamId(), rejectReasonSet);
        ItemRejectReason itemReject = (rejectReasonSet.iterator().hasNext() ? rejectReasonSet.iterator().next() : ItemRejectReason.NONE);
        if (itemReqInfo == null && itemReject == ItemRejectReason.NONE)
        {
            //No matching items. This is a new request.
            rejectReasonSet.clear();
            itemReqInfo = getNewItemReqInfo(chnl, msg, msg.streamId(), rejectReasonSet);
        }

        if (itemReqInfo == null)
        {
            return sendItemRequestReject(chnl, msg.streamId(), domainType, rejectReasonSet.iterator().next(), isPrivateStream, error);
        }

        if (((RequestMsg)msg).checkPrivateStream())
        {
            System.out.println("Received Private Stream Item Request for " + itemReqInfo.itemName + "(streamId=" + msg.streamId() + ") on domain " + DomainTypes.toString(itemReqInfo.domainType));
        }
        else
        {
            System.out.println("Received Item Request for " + itemReqInfo.itemName + "(streamId=" + msg.streamId() + ") on domain " + DomainTypes.toString(itemReqInfo.domainType));
        }

        //send item refresh
        if (!((RequestMsg)msg).checkNoRefresh())
        {
            itemReqInfo.itemInfo.isRefreshRequired = true;
            int ret = sendItemResponse(itemReqInfo, error);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (!itemReqInfo.isStreamingRequest) 
        {
            //snapshot request - so we dont have to send updates
            //free item request info
            freeItemReqInfo(itemReqInfo);
        }
        else
        {
            itemReqInfo.itemInfo.isRefreshRequired = false;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private int sendItemRequestReject(Channel channel, int streamId, int domainType, ItemRejectReason reason, boolean isPrivateStream, Error error)
    {
        //get a buffer for the item request reject status
        TransportBuffer msgBuf = channel.getBuffer(REJECT_MSG_SIZE, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        _marketPriceStatus.clear();
        _marketPriceStatus.applyHasState();
        _marketPriceStatus.state().streamState(StreamStates.CLOSED_RECOVER);
        _marketPriceStatus.state().dataState(DataStates.SUSPECT);
        if (isPrivateStream)
            _marketPriceStatus.applyPrivateStream();

        //encode request reject status
        switch (reason)
        {
            case ITEM_COUNT_REACHED:
                _marketPriceStatus.state().code(StateCodes.TOO_MANY_ITEMS);
                _marketPriceStatus.state().text().data("Item request rejected for stream id " + streamId + "- item count reached for this channel");
                break;
            case INVALID_SERVICE_ID:
                _marketPriceStatus.state().code(StateCodes.USAGE_ERROR);
                _marketPriceStatus.state().streamState(StreamStates.CLOSED);
                _marketPriceStatus.state().text().data("Item request rejected for stream id " + streamId + "- service id invalid");
                break;
            case QOS_NOT_SUPPORTED:
                _marketPriceStatus.state().code(StateCodes.USAGE_ERROR);
                _marketPriceStatus.state().streamState(StreamStates.CLOSED);
                _marketPriceStatus.state().text().data("Item request rejected for stream id " + streamId + "- QoS not supported");
                break;
            case ITEM_ALREADY_OPENED:
                _marketPriceStatus.state().code(StateCodes.ALREADY_OPEN);
                _marketPriceStatus.state().streamState(StreamStates.CLOSED);
                _marketPriceStatus.state().text().data("Item request rejected for stream id " + streamId + "- item already open with exact same key on another stream");
                break;
            case STREAM_ALREADY_IN_USE:
                _marketPriceStatus.state().code(StateCodes.USAGE_ERROR);
                _marketPriceStatus.state().streamState(StreamStates.CLOSED);
                _marketPriceStatus.state().text().data("Item request rejected for stream id " + streamId + "- stream already in use with a different key");
                break;
            case KEY_ENC_ATTRIB_NOT_SUPPORTED:
                _marketPriceStatus.state().code(StateCodes.USAGE_ERROR);
                _marketPriceStatus.state().streamState(StreamStates.CLOSED);
                _marketPriceStatus.state().text().data("Item request rejected for stream id " + streamId + "- this provider does not support key attribute information");
                break;
            case ITEM_NOT_SUPPORTED:
                _marketPriceStatus.state().code(StateCodes.USAGE_ERROR);
                _marketPriceStatus.state().streamState(StreamStates.CLOSED);
                _marketPriceStatus.state().text().data("Item request rejected for stream id " + streamId + "- item not supported");
                break;
            case PRIVATE_STREAM_REDIRECT:
                _marketPriceStatus.applyPrivateStream();
                _marketPriceStatus.state().code(StateCodes.NONE);
                _marketPriceStatus.state().streamState(StreamStates.REDIRECTED);
                _marketPriceStatus.state().text().data("Standard stream redirect to private for stream id " + streamId + " - this item must be requested via private stream");
                break;
            case PRIVATE_STREAM_MISMATCH:
                _marketPriceStatus.applyPrivateStream();
                _marketPriceStatus.state().code(StateCodes.USAGE_ERROR);
                _marketPriceStatus.state().streamState(StreamStates.CLOSED);
                _marketPriceStatus.state().text().data("Rejected request for stream id " + streamId + " - reissue via batch request is not allowed");
                break;
            //APIQA
            case TEMPORARY_REJECTED:
                _marketPriceStatus.applyPrivateStream();
                _marketPriceStatus.state().code(StateCodes.ERROR);
                _marketPriceStatus.state().streamState(StreamStates.CLOSED);
                _marketPriceStatus.state().text().data("Rejected request for stream id " + streamId + " - temporary for testing");
                break;
            default:
                break;
        }
        _encodeIter.clear();
        _marketPriceStatus.streamId(streamId);
        _marketPriceStatus.domainType(domainType);
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = _marketPriceStatus.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("MarketPriceStatus.encode() failed");
            return ret;
        }

        System.out.println("Rejecting Item Request with streamId=" + streamId + " and domain " + DomainTypes.toString(domainType) + ".  Reason: " + ItemRejectReason.toString(reason));

        return _providerSession.write(channel, msgBuf, error);
    }

    private void closeStream(Channel chnl, int streamId)
    {
        ItemRequestInfo itemRequestInfo = _itemRequestWatchList.get(chnl, streamId);
        //remove original item request information
        if (itemRequestInfo != null)
        {
            System.out.println("Closing item stream id " + itemRequestInfo.streamId + " with item name: " + itemRequestInfo.itemName);
            freeItemReqInfo(itemRequestInfo);
        }
        else
        {
            System.out.println("No item found for StreamId: " + streamId);
        }
    }

    private void deleteSymbolListItem(ItemRequestInfo itemReqInfo)
    {
        Buffer delItem = itemReqInfo.itemName;

        // TRI and RES-DS are always present in our symbol list and should never
        // be deleted
        if (_triItemName.equals(delItem) || _privateStreamItemName.equals(delItem))
        {
            return;
        }

        //search the symbol list, and delete the item if the interest count is 0
        for (int i = 2; i < SymbolListItems.MAX_SYMBOL_LIST_SIZE; i++)
        {
            if (delItem.equals(_symbolListItemWatchList.symbolListItemName(i)))
            {
                _symbolListItemWatchList.decrementInterestCount(i);
                
                //no more interest in the item, so remove it from the symbol list
                if (_symbolListItemWatchList.interestCount(i) == 0)
                {
                    _symbolListItemWatchList.clear(i);
                    _symbolListItemWatchList.decrementItemCount();

                    //find all consumers using the symbol list domain and send them updates
                    for (ItemRequestInfo itemReqInfoL : _itemRequestWatchList)
                    {
                        //Only send Symbol List updates to active channels that have made requests
                        if (itemReqInfoL.domainType == DomainTypes.SYMBOL_LIST && itemReqInfoL.channel.state() == ChannelState.ACTIVE)
                        {
                            sendSLItemUpdates(itemReqInfoL.channel, itemReqInfo, SymbolListItems.SYMBOL_LIST_UPDATE_DELETE, itemReqInfoL.streamId);
                        }
                    }
                }
                break;
            }
        }
    }

    private void freeItemReqInfo(ItemRequestInfo itemReqInfo)
    {
        if (itemReqInfo != null)
        {
            //decrement item interest count
            if (itemReqInfo.itemInfo != null && itemReqInfo.itemInfo.interestCount > 0)
            {
                itemReqInfo.itemInfo.interestCount--;
            }

            if (itemReqInfo.domainType != DomainTypes.SYMBOL_LIST && itemReqInfo.itemInfo.interestCount == 0)
            {
                deleteSymbolListItem(itemReqInfo);
            }

            //free item information if no more interest
            if (itemReqInfo.itemInfo.interestCount == 0)
            {
                itemReqInfo.itemInfo.clear();
            }

            //free item request information
            itemReqInfo.clear();
        }
    }

    /**
     * Sends the item close status message(s) for a channel. This consists of
     * finding all request information for this channel and sending the close
     * status messages to the channel.
     * 
     * @param channel - The channel to send close status message(s) to
     * @param error the error
     * @return the int
     */
    public int sendCloseStatusMsgs(Channel channel, Error error)
    {
        int ret = 0;
        for (ItemRequestInfo itemRequestInfo : _itemRequestWatchList)
        {
            if (itemRequestInfo.isInUse && itemRequestInfo.channel == channel)
            {
                ret = sendCloseStatus(channel, itemRequestInfo, error);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }

        return CodecReturnCodes.SUCCESS;
    }

    private int sendCloseStatus(Channel channel, ItemRequestInfo itemReqInfo, Error error)
    {
        //get a buffer for the close status
        TransportBuffer msgBuf = channel.getBuffer(1024, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        //encode close status
        _marketPriceStatus.clear();
        _marketPriceStatus.streamId(itemReqInfo.streamId);
        _marketPriceStatus.domainType(itemReqInfo.domainType);
        if (itemReqInfo.isPrivateStreamRequest)
            _marketPriceStatus.applyPrivateStream();
        _marketPriceStatus.applyHasState();
        _marketPriceStatus.state().streamState(StreamStates.CLOSED);
        _marketPriceStatus.state().dataState(DataStates.SUSPECT);
        _marketPriceStatus.state().text().data("Stream closed for item: " + itemReqInfo.itemName);

        _encodeIter.clear();
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = _marketPriceStatus.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("MarketPriceStatus.encode() failed");
            return ret;
        }

        return _providerSession.write(channel, msgBuf, error);
    }

    private int encodeAck(TransportBuffer msgBuf, Channel chnl, PostMsg postMsg, int nakCode, String text, Error error)
    {
        //set-up message 
        _ackMsg.msgClass(MsgClasses.ACK);
        _ackMsg.streamId(postMsg.streamId());
        _ackMsg.domainType(postMsg.domainType());
        _ackMsg.containerType(DataTypes.NO_DATA);
        _ackMsg.flags(AckMsgFlags.NONE);
        _ackMsg.nakCode(nakCode);
        _ackMsg.ackId(postMsg.postId());
        _ackMsg.seqNum(postMsg.seqNum());

        if (nakCode != NakCodes.NONE)
            _ackMsg.applyHasNakCode();

        if (postMsg.checkHasSeqNum())
            _ackMsg.applyHasSeqNum();

        if (text != null)
        {
            _ackMsg.applyHasText();
            _ackMsg.text().data(text);
        }

        //encode message
        _encodeIter.clear();

        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = _ackMsg.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("AckMsg.encode() failed");
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private ItemRequestInfo getMatchingItemReqInfo(Channel channel, Msg msg, int streamId, EnumSet<ItemRejectReason> rejectReason)
    {
        for (ItemRequestInfo itemRequestInfo : _itemRequestWatchList)
        {
            if (itemRequestInfo.isInUse && itemRequestInfo.channel == channel)
            {
                if (itemRequestInfo.domainType == msg.domainType()
                        && (itemRequestInfo.msgKey.equals(msg.msgKey())))
                {
                    //The request has the same domain and key as one currently
                    //open for this channel.
                    if (itemRequestInfo.streamId != streamId)
                    {
                        //The request has a different stream ID, meaning it
                        //would open the same item on another stream. This is
                        //not allowed(except for private streams).
                        if (!((RequestMsg)msg).checkPrivateStream())
                        {
                            rejectReason.add(ItemRejectReason.ITEM_ALREADY_OPENED);
                            return null;
                        }
                        //Otherwise continue checking the list
                    }
                    else
                    {
                        //Check that the private stream flag matches correctly.
                        if (((RequestMsg)msg).checkPrivateStream() && !itemRequestInfo.isPrivateStreamRequest
                                || !(((RequestMsg)msg).checkPrivateStream()) && itemRequestInfo.isPrivateStreamRequest)
                        {
                            //This item would be a match except that the
                            //private stream flag does not match.
                            rejectReason.add(ItemRejectReason.PRIVATE_STREAM_MISMATCH);
                            return null;
                        }

                        //The domain, key, stream ID, and private stream flag
                        //all match, so this item is a match, and the request
                        //is a reissue.
                        return itemRequestInfo;
                    }
                }
                else if (itemRequestInfo.streamId == streamId)
                {
                    //This stream ID is already in use for a different item.
                    rejectReason.add(ItemRejectReason.STREAM_ALREADY_IN_USE);
                    return null;
                }
            }
        }

        rejectReason.add(ItemRejectReason.NONE);
        return null;
    }

    private int sendAck(Channel chnl, PostMsg postMsg, int nakCode, String errText, Error error)
    {
        //send an ack if it was requested
        if (postMsg.checkAck())
        {
            TransportBuffer msgBuf = chnl.getBuffer(ACK_MSG_SIZE, false, error);
            if (msgBuf == null)
            {
                return CodecReturnCodes.FAILURE;
            }

            int ret = encodeAck(msgBuf, chnl, postMsg, nakCode, errText, error);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            return _providerSession.write(chnl, msgBuf, error);
        }

        return CodecReturnCodes.SUCCESS;
    }
    //APIQA
    // Add count variable to reject every 5th request.
    public int rejectItemCount = 0;
    /*
     * Initializes a new item request information structure for a channel (or
     * rejects the request if its channel has too many items open).
     */
    private ItemRequestInfo getNewItemReqInfo(Channel channel, Msg msg, int stream, EnumSet<ItemRejectReason> rejectReasons)
    {
        //APIQA
        if (++rejectItemCount%5 == 0) {
              rejectReasons.add(ItemRejectReason.TEMPORARY_REJECTED);
            return null;
        } 
        ItemRequestInfo itemRequestInfo = null;
        int count = 0;

        
        //Find an available item request info structure to use, and check that
        //the channel has not reached its allowed limit of open items.
        for (ItemRequestInfo itemReqInfo : _itemRequestWatchList)
        {
            if (itemReqInfo.isInUse)
            {
                if (itemReqInfo.channel == channel)
                {
                    ++count;
                    if (count >= ProviderDirectoryHandler.OPEN_LIMIT)
                    {
                        //Consumer has requested too many items.
                        rejectReasons.add(ItemRejectReason.ITEM_COUNT_REACHED);
                        return null;
                    }
                }
            }
            else if (itemRequestInfo == null)
            {
                itemRequestInfo = itemReqInfo;
                break;
            }
        }

        if (itemRequestInfo == null)
        {
            rejectReasons.add(ItemRejectReason.ITEM_COUNT_REACHED);
            return null;
        }

        itemRequestInfo.channel = channel;
        itemRequestInfo.isInUse = true;
        if (copyMsgKey(itemRequestInfo.msgKey, msg.msgKey()) != CodecReturnCodes.SUCCESS)
        {
            rejectReasons.add(ItemRejectReason.ITEM_NOT_SUPPORTED);
            return null;
        }

        itemRequestInfo.domainType = msg.domainType();
        //copy item name buffer
        ByteBuffer byteBuffer = ByteBuffer.allocate(itemRequestInfo.msgKey.name().length());
        itemRequestInfo.msgKey.name().copy(byteBuffer);
        itemRequestInfo.itemName.data(byteBuffer);
        int msgFlags = msg.flags();
        if ( (msgFlags & RequestMsgFlags.PRIVATE_STREAM) != 0)
    	{
    		itemRequestInfo.isPrivateStreamRequest(true);
    	}

    	/* get IsStreamingRequest */
    	if ((msgFlags & RequestMsgFlags.STREAMING) != 0)
    	{
    		itemRequestInfo.isStreamingRequest(true);
    	}

    	/* get IncludeKeyInUpdates */
    	if ((msgFlags & RequestMsgFlags.MSG_KEY_IN_UPDATES) != 0)
    	{
    		itemRequestInfo.includeKeyInUpdates(true);
    	}
    	
        //get item information
        itemRequestInfo.itemInfo = _itemInfoWatchList.get(channel, itemRequestInfo.itemName,
        		itemRequestInfo.domainType, itemRequestInfo.isPrivateStreamRequest());
        if (itemRequestInfo.itemInfo == null)
        {
            rejectReasons.add(ItemRejectReason.ITEM_NOT_SUPPORTED);
        }
        else
        {
            switch (itemRequestInfo.domainType)
            {
                case DomainTypes.MARKET_PRICE:
                    itemRequestInfo.itemInfo.itemData = _marketPriceItemWatchList.get(itemRequestInfo.itemName.toString());
                    break;
                case DomainTypes.MARKET_BY_ORDER:
                    itemRequestInfo.itemInfo.itemData = _marketByOrderItemWatchList.get(itemRequestInfo.itemName.toString());
                    break;
                case DomainTypes.MARKET_BY_PRICE:
                    itemRequestInfo.itemInfo.itemData = _marketByPriceItemWatchList.get(itemRequestInfo.itemName.toString());
                    break;
                case DomainTypes.SYMBOL_LIST:
                    break;
                default:
                    break;
            }

            if ((itemRequestInfo.itemInfo.itemData == null) && (itemRequestInfo.domainType != DomainTypes.SYMBOL_LIST))
            {
                rejectReasons.add(ItemRejectReason.ITEM_COUNT_REACHED);
                return null;
            }

            if (itemRequestInfo.itemInfo.domainType != DomainTypes.SYMBOL_LIST)
            {
                addSymbolListItem(channel, itemRequestInfo);
            }
        }
        //get IsStreamingRequest
        if (((RequestMsg)msg).checkStreaming())
        {
            itemRequestInfo.isStreamingRequest = true;
        }

        //IsPrivateStreamRequest
        if (((RequestMsg)msg).checkPrivateStream())
        {
            itemRequestInfo.isPrivateStreamRequest = true;
        }

        //IncludeKeyInUpdates
        if (((RequestMsg)msg).checkMsgKeyInUpdates())
        {
            itemRequestInfo.includeKeyInUpdates = true;
        }

        //increment item interest count if new request
        itemRequestInfo.itemInfo.interestCount++;
        itemRequestInfo.streamId = stream;

        //provide a refresh if one was requested.
        itemRequestInfo.itemInfo.isRefreshRequired = (((RequestMsg)msg).checkNoRefresh()) ? false : true;

        return itemRequestInfo;
    }

    private void addSymbolListItem(Channel channel, ItemRequestInfo itemReqInfo)
    {
        int itemVacancy = 0;
        Buffer newItem = itemReqInfo.itemName;
        boolean foundVacancy = false;

        //TRI and RES-DS are added to our symbol list at initialization, and
        //are always present so they never need to be added again
        if (_triItemName.equals(newItem) || _privateStreamItemName.equals(newItem) || _itemCount >= SymbolListItems.MAX_SYMBOL_LIST_SIZE)
        {
            return;
        }

        //check to see if this item is already in the item list
        for (int i = 2; i < SymbolListItems.MAX_SYMBOL_LIST_SIZE; i++)
        {
            //if the item is already present, increment the interest count
            if (newItem.equals(_symbolListItemWatchList.symbolListItemName(i)))
            {
                _symbolListItemWatchList.incrementInterestCount(i);
                return;
            }
            if ((_symbolListItemWatchList.getStatus(i) == false) && foundVacancy == false)
            {
                //store the index of the first vacancy in the symbol list
                foundVacancy = true;
                itemVacancy = i;
            }
        }

        //add the new item name to the symbol list 
        _symbolListItemWatchList.symbolListItemName(newItem, itemVacancy);
        _symbolListItemWatchList.incrementInterestCount(itemVacancy);

        //find all consumers currently using the symbol list domain, and send
        //them updates
        for (ItemRequestInfo itemReqInfoL : _itemRequestWatchList)
        {
            if (itemReqInfoL.domainType == DomainTypes.SYMBOL_LIST)
            {
                sendSLItemUpdates(itemReqInfoL.channel, itemReqInfo, SymbolListItems.SYMBOL_LIST_UPDATE_ADD, itemReqInfoL.streamId);
            }
        }
    }

    private void sendSLItemUpdates(Channel chnl, ItemRequestInfo itemReqInfo, int responseType, int streamId)
    {
        //get a buffer for the response
        TransportBuffer msgBuf = chnl.getBuffer(SymbolListItems.MAX_SYMBOL_LIST_SIZE, false, _error);
        if (msgBuf == null)
        {
            System.out.println("chnl.getBuffer(): Failed " + _error.text());
            return;
        }

        int ret = _symbolListItemWatchList.encodeResponse(chnl, itemReqInfo.itemInfo, msgBuf, streamId, true, serviceId(), itemReqInfo.isStreamingRequest, _dictionaryHandler.dictionary(), responseType, _error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("encodeSymbolListResponse() failed");
        }

        if (_providerSession.write(chnl, msgBuf, _error) == TransportReturnCodes.FAILURE)
            System.out.println("Error writing message: " + _error.text());
    }

    /**
     * Sends the item update(s) for a channel. This consists of finding all
     * request information for this channel, and sending the responses to the
     * channel.
     * 
     * @param channel - The channel of the response
     * @param error - error information in case of failure.
     * @return returns success if sending of item updates succeeds or failure if
     *         it fails.
     */
    public int sendItemUpdates(Channel channel, Error error)
    {
        int ret;
        for (ItemRequestInfo itemReqInfo : _itemRequestWatchList)
        {
            if (itemReqInfo.isInUse && itemReqInfo.channel == channel)
            {
                ret = sendItemResponse(itemReqInfo, error);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        return CodecReturnCodes.SUCCESS;
    }

    private int sendItemResponse(ItemRequestInfo itemReqInfo, Error error)
    {
        //market by price is handled separately due to multi-part refresh
        if (itemReqInfo.domainType == DomainTypes.MARKET_BY_PRICE)
        {
            return sendMBPItemResponse(itemReqInfo, error);
        }

        //get a buffer for the response
        TransportBuffer msgBuf = itemReqInfo.channel.getBuffer(ITEM_MSG_SIZE, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        //Encode the message with data appopriate for the domain
        switch (itemReqInfo.domainType)
        {
            case DomainTypes.MARKET_PRICE:
                //encode market price response 
                int ret = _marketPriceItemWatchList.encodeResponse(itemReqInfo.channel, itemReqInfo.itemInfo, msgBuf, true, itemReqInfo.streamId, itemReqInfo.isStreamingRequest, itemReqInfo.isPrivateStreamRequest, serviceId(), _dictionaryHandler.dictionary(), error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
                break;
            case DomainTypes.MARKET_BY_ORDER:
                //encode market by order response
                ret = _marketByOrderItemWatchList.encodeResponse(itemReqInfo.channel, itemReqInfo.itemInfo, msgBuf, true, itemReqInfo.streamId, itemReqInfo.isStreamingRequest, itemReqInfo.isPrivateStreamRequest, serviceId(), _dictionaryHandler.dictionary(), error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
                break;
            case DomainTypes.SYMBOL_LIST:
                //encode symbol list response
                //only encode refresh responses for the symbol list from this
                //method. symbol list update responses are handled separately
                if (itemReqInfo.itemInfo.isRefreshRequired)
                {
                    ret = _symbolListItemWatchList.encodeResponse(itemReqInfo.channel, itemReqInfo.itemInfo, msgBuf, itemReqInfo.streamId, true, serviceId(), itemReqInfo.isStreamingRequest, _dictionaryHandler.dictionary(), SymbolListItems.SYMBOL_LIST_REFRESH, error);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                }
                else
                {
                    itemReqInfo.channel.releaseBuffer(msgBuf, error);
                    return CodecReturnCodes.SUCCESS;
                }
                break;
            default:
                error.text("Received unhandled domain " + itemReqInfo.domainType + " for item ");
                return CodecReturnCodes.FAILURE;
        }

        //send item response
        return _providerSession.write(itemReqInfo.channel, msgBuf, error);
    }

    private int sendMBPItemResponse(ItemRequestInfo itemReqInfo, Error error)
    {
        MarketByPriceItem mbpItem = new MarketByPriceItem();

        if (itemReqInfo.itemInfo.isRefreshRequired)
        {
            mbpItem = (MarketByPriceItem)itemReqInfo.itemInfo.itemData;
            for (int i = 0; i < MAX_REFRESH_PARTS; i++)
            {
                //get a buffer for the response
                TransportBuffer msgBuf = itemReqInfo.channel.getBuffer(ITEM_MSG_SIZE, false, error);
                if (msgBuf == null)
                    return CodecReturnCodes.FAILURE;

                if (msgBuf != null)
                {
                    //encode market by price refresh 
                    int ret = _marketByPriceItemWatchList.encodeRefresh(itemReqInfo.channel, itemReqInfo.itemInfo, msgBuf, true, itemReqInfo.streamId, itemReqInfo.isStreamingRequest, itemReqInfo.isPrivateStreamRequest, serviceId(), _dictionaryHandler.dictionary(), i, error);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }

                    //send item response
                    ret = _providerSession.write(itemReqInfo.channel, msgBuf, error);
                    if (ret != TransportReturnCodes.SUCCESS)
                        return ret;
                }

                //send an update between each part of the refresh
                if (i < MAX_REFRESH_PARTS - 1)
                {
                    mbpItem.priceInfoList.get(0).ORDER_SIZE.value(mbpItem.priceInfoList.get(0).ORDER_SIZE.toDouble() + i + 1, mbpItem.priceInfoList.get(0).ORDER_SIZE.hint()); // change
                                                                                                                                                                               // order
                    //size for update
                    mbpItem.priceInfoList.get(1).ORDER_SIZE.value(mbpItem.priceInfoList.get(1).ORDER_SIZE.toDouble() + i + 1, mbpItem.priceInfoList.get(1).ORDER_SIZE.hint()); // change
                    mbpItem.priceInfoList.get(2).ORDER_SIZE.value(mbpItem.priceInfoList.get(2).ORDER_SIZE.toDouble() + i + 1, mbpItem.priceInfoList.get(2).ORDER_SIZE.hint()); // change
                                                                                                                                                                               // order
                    //get a buffer for the response
                    msgBuf = itemReqInfo.channel.getBuffer(ITEM_MSG_SIZE, false, error);
                    if (msgBuf == null)
                        return CodecReturnCodes.FAILURE;
                    //encode market by price update
                    int ret = _marketByPriceItemWatchList.encodeUpdate(itemReqInfo.channel, itemReqInfo.itemInfo, msgBuf, true, itemReqInfo.streamId, itemReqInfo.isStreamingRequest, itemReqInfo.isPrivateStreamRequest, serviceId(), _dictionaryHandler.dictionary(), error);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }

                    //send item response
                    ret = _providerSession.write(itemReqInfo.channel, msgBuf, error);
                    if (ret != TransportReturnCodes.SUCCESS)
                    {
                        return ret;
                    }

                    mbpItem.priceInfoList.get(0).ORDER_SIZE.value(mbpItem.priceInfoList.get(0).ORDER_SIZE.toDouble() - (i + 1), mbpItem.priceInfoList.get(0).ORDER_SIZE.hint()); // change
                                                                                                                                                                                 // order
                    // size for update
                    mbpItem.priceInfoList.get(1).ORDER_SIZE.value(mbpItem.priceInfoList.get(1).ORDER_SIZE.toDouble() - (i + 1), mbpItem.priceInfoList.get(1).ORDER_SIZE.hint()); // change
                                                                                                                                                                                 // order
                    mbpItem.priceInfoList.get(2).ORDER_SIZE.value(mbpItem.priceInfoList.get(2).ORDER_SIZE.toDouble() - (i + 1), mbpItem.priceInfoList.get(2).ORDER_SIZE.hint()); // change
                }
            }
        }
        else
        //update
        {
            //get a buffer for the response
            TransportBuffer msgBuf = itemReqInfo.channel.getBuffer(ITEM_MSG_SIZE, false, error);
            if (msgBuf == null)
                return CodecReturnCodes.FAILURE;

            //encode market by price update
            int ret = _marketByPriceItemWatchList.encodeUpdate(itemReqInfo.channel, itemReqInfo.itemInfo, msgBuf, true, itemReqInfo.streamId, itemReqInfo.isStreamingRequest, itemReqInfo.isPrivateStreamRequest, serviceId(), _dictionaryHandler.dictionary(), error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            //send item response
            return _providerSession.write(itemReqInfo.channel, msgBuf, error);
        }

        return CodecReturnCodes.SUCCESS;
    }

    private int copyMsgKey(MsgKey destKey, MsgKey sourceKey)
    {
        destKey.flags(sourceKey.flags());
        destKey.nameType(sourceKey.nameType());
        if (sourceKey.checkHasName() && sourceKey.name() != null)
        {
            destKey.name().data(ByteBuffer.allocate(sourceKey.name().length()));
            sourceKey.name().copy(destKey.name());
        }
        destKey.serviceId(sourceKey.serviceId());
        destKey.filter(sourceKey.filter());
        destKey.identifier(sourceKey.identifier());
        destKey.attribContainerType(sourceKey.attribContainerType());
        if (sourceKey.checkHasAttrib() && sourceKey.encodedAttrib() != null)
        {
            int ret = sourceKey.encodedAttrib().copy(destKey.encodedAttrib());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private boolean isStreamInUse(Channel chnl, int streamId, MsgKey key)
    {
        boolean streamInUse = false;

        for (ItemRequestInfo itemReqInfo : _itemRequestWatchList)
        {
            if (itemReqInfo.isInUse &&
                    itemReqInfo.channel == chnl &&
                    itemReqInfo.streamId == streamId)
            {
                if (itemReqInfo.msgKey.equals(key))
                {
                    streamInUse = true;
                    break;
                }
            }
        }

        return streamInUse;
    }

    /*
     * Processes a posting request. This consists of decoding the
     * status/update/refresh information and sending it out to all clients the
     * posting request may be on-stream or off-stream
     * 
     * In this example, we will send the post to any stream which has the item
     * open. We will also update the item's field values with the post values.
     * if the item name in an off-steam post is not open on any streams, then no
     * updates will be sent or made. a more complete implementation might choose
     * to add unknown(or new) items to the item cache if the client has
     * sufficient postUserRights a more complete implementation might also
     * choose to add unknown(or new) fields on a posting refresh to the item
     * cache.
     */
    private int processPost(Channel chnl, Msg msg, DecodeIterator dIter, Error error)
    {
        LoginRequestInfo loginRequestInfo;
        PostMsg postMsg = (PostMsg)msg;

        // get the login stream so that we can see if the post was an off-stream
        // post
        if ((loginRequestInfo = _loginHandler.findLoginRequestInfo(chnl)) == null)
        {
            return sendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, "Received a post message request from client before login", error);
        }

        ItemInfo itemInfo;
        // if the post is on the login stream, then it's an off-stream post
        if (loginRequestInfo.loginRequest().streamId() == msg.streamId())
        {
            // the msg key must be specified to provide the item name
            if (!postMsg.checkHasMsgKey())
            {
                return sendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, "Received an off-stream post message request from client without a msgkey", error);
            }
            System.out.println("Received an off-stream item post (item=" + postMsg.msgKey().name() + ")");
            // look up the item name
            // for this example, we will treat an unknown item as an error
            // However, other providers may choose to add the item to their
            // cache
            if ((itemInfo = _itemInfoWatchList.get(postMsg.msgKey().name(), postMsg.domainType(), false)) == null)
            {
                return sendAck(chnl, postMsg, NakCodes.SYMBOL_UNKNOWN, "Received an off-stream post message for an unknown item", error);
            }
        }
        else
        {
            ItemRequestInfo itemReqInfo = null;
            // the msgkey is not required for on-stream post
            // get the item request associated with this on-stream post
            if ((itemReqInfo = _itemRequestWatchList.get(chnl, postMsg.streamId())) == null)
            {
                return sendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, "Received an on-stream post message on a stream that does not have an item open", error);
            }

            itemInfo = itemReqInfo.itemInfo;
            System.out.println("Received an on-stream post for item= " + itemInfo.itemName);
        }
        
        // This is used to indicate whether an ACK message is sent.
        boolean sendAck = false;
        
        // if the post message contains another message, then use the
        // "contained" message as the update/refresh/status
        if (postMsg.containerType() == DataTypes.MSG)
        {
            _nestedMsg.clear();
            int ret = _nestedMsg.decode(dIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("Unable to decode msg");
                return ret;
            }
            switch (_nestedMsg.msgClass())
            {
                case MsgClasses.REFRESH:
                    _nestedMsg.msgClass(MsgClasses.REFRESH);
                    int flags = _nestedMsg.flags();
                    flags |= RefreshMsgFlags.HAS_POST_USER_INFO;
                    flags &= ~RefreshMsgFlags.SOLICITED;
                    _nestedMsg.flags(flags);
                 
                    ((RefreshMsg)_nestedMsg).postUserInfo().userAddr(postMsg.postUserInfo().userAddr());
                    ((RefreshMsg)_nestedMsg).postUserInfo().userId(postMsg.postUserInfo().userId());
                    if (updateItemInfoFromPost(itemInfo, _nestedMsg, dIter, error) != CodecReturnCodes.SUCCESS)
                    {
                        ret = sendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, error.text(), error);
                        if (ret != CodecReturnCodes.SUCCESS)
                        {
                            return ret;
                        }
                        
                        sendAck = true;
                    }
                
                    break;

                case MsgClasses.UPDATE:
                    _nestedMsg.msgClass(MsgClasses.UPDATE);
                    ((UpdateMsg)_nestedMsg).flags(_nestedMsg.flags() | UpdateMsgFlags.HAS_POST_USER_INFO);
                    ((UpdateMsg)_nestedMsg).postUserInfo().userAddr(postMsg.postUserInfo().userAddr());
                    ((UpdateMsg)_nestedMsg).postUserInfo().userId(postMsg.postUserInfo().userId());
                    if (updateItemInfoFromPost(itemInfo, _nestedMsg, dIter, error) != CodecReturnCodes.SUCCESS)
                    {
                        ret = sendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, error.text(), error);
                        if (ret != CodecReturnCodes.SUCCESS)
                        {
                            return ret;
                        }
                        
                        sendAck = true;
                    }
                    break;

                case MsgClasses.STATUS:
                    _nestedMsg.msgClass(MsgClasses.STATUS);
                    ((StatusMsg)_nestedMsg).flags(_nestedMsg.flags() | StatusMsgFlags.HAS_POST_USER_INFO);
                    ((StatusMsg)_nestedMsg).postUserInfo().userAddr(postMsg.postUserInfo().userAddr());
                    ((StatusMsg)_nestedMsg).postUserInfo().userId(postMsg.postUserInfo().userId());
                    if (((StatusMsg)_nestedMsg).checkHasState() && ((StatusMsg)_nestedMsg).state().streamState() == StreamStates.CLOSED)
                    {
                        // check if the user has the rights to send a post that
                        // closes an item
                        if (postMsg.checkHasPostUserRights() || postMsg.postUserRights() == 0)
                        {
                            ret = sendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, "client has insufficient rights to close/delete an item", error);
                            if (ret != CodecReturnCodes.SUCCESS)
                                return ret;
                            
                            sendAck = true;
                        }
                    }
                    break;
                default:
                	break;
            }
        }
        else
        {
            //It's a container(e.g. field list). Add an update header for reflecting.
            _updateMsg.clear();
            _updateMsg.msgClass(MsgClasses.UPDATE);
            _updateMsg.domainType(postMsg.domainType());
            _updateMsg.containerType(postMsg.containerType());
            if (msg.encodedDataBody() != null && msg.encodedDataBody().length() > 0)
                _updateMsg.encodedDataBody(msg.encodedDataBody());
            _updateMsg.flags(UpdateMsgFlags.HAS_POST_USER_INFO);
            _updateMsg.postUserInfo().userAddr(postMsg.postUserInfo().userAddr());
            _updateMsg.postUserInfo().userId(postMsg.postUserInfo().userId());
            if (postMsg.checkHasMsgKey())
            {
                _updateMsg.flags(_updateMsg.flags() | UpdateMsgFlags.HAS_MSG_KEY);
                _updateMsg.msgKey().copy(postMsg.msgKey());
            }

            if (updateItemInfoFromPost(itemInfo, msg, dIter, error) != CodecReturnCodes.SUCCESS)
            {
                int ret = sendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, error.text(), error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }

                sendAck = true;
            }
        }
        
        int ret;

        if(sendAck == false)
        {
        	ret = sendAck(chnl, postMsg, NakCodes.NONE, null, error);
        	if (ret != CodecReturnCodes.SUCCESS)
        	{
        		return ret;
        	}
        }
        // send the post to all public streams with this item open
        for (ItemRequestInfo itemReqInfoL : _itemRequestWatchList)
        {
            if (itemReqInfoL.itemInfo == itemInfo)
            {
                _encodeIter.clear();
                TransportBuffer sendBuf = itemReqInfoL.channel.getBuffer(POST_MSG_SIZE, false, error);
                if (sendBuf == null)
                {
                    return CodecReturnCodes.FAILURE;
                }
                ret = _encodeIter.setBufferAndRWFVersion(sendBuf, itemReqInfoL.channel.majorVersion(), itemReqInfoL.channel.minorVersion());
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
                    return CodecReturnCodes.FAILURE;
                }

                if (postMsg.containerType() == DataTypes.MSG)
                {
                     // send the contained/embedded message if there was one.
                    _nestedMsg.streamId(itemReqInfoL.streamId);
                    if(_nestedMsg.msgClass() == MsgClasses.REFRESH)
                    {
                        ((RefreshMsg)_nestedMsg).applyHasMsgKey();
                    }
                    ret = _nestedMsg.encode(_encodeIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("nestedMsg.encode() failed");
                        return CodecReturnCodes.FAILURE;
                    }
                    ret = _providerSession.write(itemReqInfoL.channel, sendBuf, error);
                    if (ret != TransportReturnCodes.SUCCESS)
                        return CodecReturnCodes.FAILURE;

                    // check if its a status close and close any open streams if
                    // it is
                    if (_nestedMsg.msgClass() == MsgClasses.STATUS && ((StatusMsg)_nestedMsg).checkHasState() && ((StatusMsg)_nestedMsg).state().streamState() == StreamStates.CLOSED)
                        closeStream(itemReqInfoL.channel, _nestedMsg.streamId());
                }
                else
                {
                    // send an update message if the post contained data
                    _updateMsg.streamId(itemReqInfoL.streamId);
                    ret = _updateMsg.encode(_encodeIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("nestedMsg.encode() failed");
                        return CodecReturnCodes.FAILURE;
                    }
                    ret = _providerSession.write(itemReqInfoL.channel, sendBuf, error);
                    if (ret != TransportReturnCodes.SUCCESS)
                        return CodecReturnCodes.FAILURE;
                }
            }
        }

        return CodecReturnCodes.SUCCESS;
    }

    /*
     * update the item info from the post based on market domain This example
     * supports posting on the market Price domain a more fully functional
     * implementation would support additional domains
     */
    private int updateItemInfoFromPost(ItemInfo itemInfo, Msg msg, DecodeIterator dIter, Error error)
    {
        int ret;

        switch (itemInfo.domainType)
        {
            case DomainTypes.MARKET_PRICE:
                ret = _marketPriceItemWatchList.updateFieldsFromPost((MarketPriceItem)itemInfo.itemData, dIter, error);
                break;

            case DomainTypes.MARKET_BY_ORDER:
            case DomainTypes.MARKET_BY_PRICE:
            default:
                error.text("Unsupported domain" + itemInfo.domainType + " in post message update/refresh");
                ret = CodecReturnCodes.FAILURE;
        }
        return ret;
    }
   
    /**
     * Closes all item requests for the closed channel.
     * 
     * @param channel - channel for which channel close has received.
     */
    public void closeRequests(Channel channel)
    {
        //find original item request information associated with channel
        for (ItemRequestInfo itemRequestInfoL : _itemRequestWatchList)
        {
            if (itemRequestInfoL.channel == channel && itemRequestInfoL.isInUse)
            {
                freeItemReqInfo(itemRequestInfoL);
            }
        }
    }

    /**
     * Updates item information for all items in the watch list.
     */
    public void updateItemInfo()
    {
        _itemInfoWatchList.update();
    }

    /**
     * Service id.
     *
     * @return the int
     */
    public int serviceId()
    {
        return _serviceId;
    }

    /**
     * Service id.
     *
     * @param serviceId the service id
     */
    public void serviceId(int serviceId)
    {
        this._serviceId = serviceId;
    }
}
