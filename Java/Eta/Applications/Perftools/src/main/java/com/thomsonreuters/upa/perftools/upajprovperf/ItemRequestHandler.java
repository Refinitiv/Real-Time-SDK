package com.thomsonreuters.upa.perftools.upajprovperf;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.PostMsg;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RefreshMsgFlags;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.shared.provider.ItemRejectReason;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceStatus;
import com.thomsonreuters.upa.perftools.common.ItemAttributes;
import com.thomsonreuters.upa.perftools.common.ItemFlags;
import com.thomsonreuters.upa.perftools.common.ItemInfo;
import com.thomsonreuters.upa.perftools.common.PerfToolsReturnCodes;
import com.thomsonreuters.upa.perftools.common.ProviderSession;
import com.thomsonreuters.upa.perftools.common.ProviderThread;
import com.thomsonreuters.upa.perftools.upajprovperf.MarketPriceDecoder;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

/**
 * Implementation of handling item requests for the upajProvPerf application.
 */
public class ItemRequestHandler
{
	private MarketPriceDecoder _marketPriceDecoder;
    private MarketPriceStatus _itemStatusMessage;
    private ItemAttributes _itemAttributes;
    private EncodeIterator _eIter;
    private Msg _tmpMsg;

    public ItemRequestHandler()
    {
        _eIter = CodecFactory.createEncodeIterator();
        _itemStatusMessage = new MarketPriceStatus();
        _itemAttributes = new ItemAttributes();
        _tmpMsg = CodecFactory.createMsg();
		_marketPriceDecoder = new MarketPriceDecoder();
    }

    /**
     * Processes item request message.
     * 
     * @param providerThread - Provider thread that received the request.
     * @param providerSession - Provider channel session.
     * @param msg - request message
     * @param openLimit - item count limit
     * @param serviceId - service id
     * @param dirQos - Qos provided by source directory
     * @param decodeIter - Decode iterator containing received message buffer
     * @param error - in case of error, this object is populated with error
     *            information
     * @return {@link PerfToolsReturnCodes#SUCCESS} for successful request
     *         processing, < {@link PerfToolsReturnCodes#SUCCESS} when request
     *         processing fails.
     */
    int processMsg(ProviderThread providerThread, ProviderSession providerSession, Msg msg, int openLimit, int serviceId, Qos dirQos, DecodeIterator decodeIter, Error error)
    {
        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
                providerThread.itemRequestCount().increment();

                // get key
                MsgKey msgKey = msg.msgKey();

                // check if item count reached
                if (providerSession.openItemCount() >= openLimit)
                {
                    return sendRequestReject(providerThread, providerSession, msg, ItemRejectReason.ITEM_COUNT_REACHED, error);
                }
                // check if service id correct
                if (msgKey.serviceId() != serviceId)
                {
                    return sendRequestReject(providerThread, providerSession, msg, ItemRejectReason.INVALID_SERVICE_ID, error);
                }
                // check if QoS supported
                if (((RequestMsg)msg).checkHasQos() && ((RequestMsg)msg).checkHasWorstQos())
                {
                    if (!dirQos.isInRange(((RequestMsg)msg).qos(), ((RequestMsg)msg).worstQos()))
                    {
                        return sendRequestReject(providerThread, providerSession, msg, ItemRejectReason.QOS_NOT_SUPPORTED, error);
                    }
                }
                else if (((RequestMsg)msg).checkHasQos())
                {
                    if (!dirQos.equals(((RequestMsg)msg).qos()))
                    {
                        return sendRequestReject(providerThread, providerSession, msg, ItemRejectReason.QOS_NOT_SUPPORTED, error);
                    }
                }

                // check if item already opened with exact same key and domain.
                // If we find one, check the StreamId.
                // If the streamId matches, it is a reissue.
                // If the streamId does not match, reject the redundant request.
                _itemAttributes.domainType(msg.domainType());
                _itemAttributes.msgKey(msgKey);
                ItemInfo itemInfo = findAlreadyOpenedItem(providerSession, msg, _itemAttributes);
                if (itemInfo != null && itemInfo.streamId() != msg.streamId())
                {
                    return sendRequestReject(providerThread, providerSession, msg, ItemRejectReason.ITEM_ALREADY_OPENED, error);
                }

                if (isStreamInUse(providerSession, msg.streamId(), msgKey))
                {
                    return sendRequestReject(providerThread, providerSession, msg, ItemRejectReason.STREAM_ALREADY_IN_USE, error);
                }

                if (itemInfo == null)
                {
                    // New request

                    // get item info structure
                    ItemAttributes itemAttributes = new ItemAttributes();
                    itemAttributes.domainType(msg.domainType());
                    itemAttributes.msgKey(CodecFactory.createMsgKey());

                    // MsgKey.copy() - shallow copy for name and attrib
                    msgKey.copy(itemAttributes.msgKey());

                    if (msgKey.checkHasName())
                    {
                        // deep copy item name buffer
                        ByteBuffer nameBytes = ByteBuffer.allocate(msgKey.name().length());
                        msgKey.name().copy(nameBytes);
                        itemAttributes.msgKey().name().data(nameBytes);
                    }

                    if (msgKey.checkHasAttrib())
                    {
                        // deep copy attrib buffer
                        ByteBuffer attribBytes = ByteBuffer.allocate(msgKey.encodedAttrib().length());
                        msgKey.encodedAttrib().copy(attribBytes);
                        itemAttributes.msgKey().encodedAttrib().data(attribBytes);
                    }

                    itemInfo = providerSession.createItemInfo(itemAttributes, msg.streamId());

                    if (itemInfo == null)
                    {
                        return PerfToolsReturnCodes.FAILURE;
                    }

                    // get StreamId
                    itemInfo.streamId(msg.streamId());
                    itemInfo.itemFlags(itemInfo.itemFlags() | ItemFlags.IS_SOLICITED);
                    providerSession.itemAttributesTable().put(itemAttributes, itemInfo);
                }
                else
                {
                    // else it was a reissue
                    if (!((RequestMsg)msg).checkNoRefresh())
                    {
                        // Move item back to refresh queue.
                        providerSession.refreshItemList().add(itemInfo);
                    }
                }

                // get IsStreamingRequest
                if (((RequestMsg)msg).checkStreaming())
                {
                    itemInfo.itemFlags(itemInfo.itemFlags() | ItemFlags.IS_STREAMING_REQ);
                }
                
                // check if the request is for a private stream
                if (((RequestMsg)msg).checkPrivateStream())
                {
                    itemInfo.itemFlags(itemInfo.itemFlags() | ItemFlags.IS_PRIVATE);
                }
                break;
            case MsgClasses.POST:
                providerThread.postMsgCount().increment();
                return reflectPostMsg(providerThread, providerSession, decodeIter, (PostMsg)msg, error);
            case MsgClasses.GENERIC:
            	if(providerThread.getProvThreadInfo().stats().firstGenMsgRecvTime() == 0)
            		providerThread.getProvThreadInfo().stats().firstGenMsgRecvTime(System.nanoTime());
                providerThread.getProvThreadInfo().stats().genMsgRecvCount().increment();
        		return processGenMsg(providerThread, decodeIter, providerSession, (GenericMsg)msg);
            	
            case MsgClasses.CLOSE:
                // close item stream
                providerThread.closeMsgCount().increment();
                providerSession.closeItemStream(msg.streamId());
                break;
            default:
                System.out.printf("\nReceived Unhandled Item Msg Class: %d\n", msg.msgClass());
                break;
        }

        return PerfToolsReturnCodes.SUCCESS;
    }
 
    // Sends a CloseMsg rejecting a request
    int sendRequestReject(ProviderThread providerThread, ProviderSession providerSession, Msg msg, ItemRejectReason reason, Error error)
    {
        int ret = providerThread.getItemMsgBuffer(providerSession, 128, error);
        if (ret < TransportReturnCodes.SUCCESS)
        {
            return PerfToolsReturnCodes.FAILURE;
        }

        ret = encodeItemReject(providerSession, msg, reason, error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return PerfToolsReturnCodes.FAILURE;
        }
        System.out.println("Rejecting Item Request with streamId=" + msg.streamId() + " and domain " + DomainTypes.toString(msg.domainType()) + ".  Reason: " + ItemRejectReason.toString(reason));

        return providerThread.sendItemMsgBuffer(providerSession, true, error);
    }

    /*
     * Encode Market price status message with item request reason.
     */
    private int encodeItemReject(ProviderSession providerSession, Msg msg, ItemRejectReason reason, Error error)
    {
        // encode request reject status
        _itemStatusMessage.clear();
        _itemStatusMessage.applyHasState();
        _itemStatusMessage.state().streamState(StreamStates.CLOSED_RECOVER);
        _itemStatusMessage.state().dataState(DataStates.SUSPECT);

        // encode request reject status
        switch (reason)
        {
            case ITEM_COUNT_REACHED:
                _itemStatusMessage.state().code(StateCodes.TOO_MANY_ITEMS);
                _itemStatusMessage.state().text().data("Item request rejected for stream id " + msg.streamId() + "- item count reached for this channel");
                break;
            case INVALID_SERVICE_ID:
                _itemStatusMessage.state().code(StateCodes.USAGE_ERROR);
                _itemStatusMessage.state().streamState(StreamStates.CLOSED);
                _itemStatusMessage.state().text().data("Item request rejected for stream id " + msg.streamId() + "- service id invalid");
                break;
            case QOS_NOT_SUPPORTED:
                _itemStatusMessage.state().code(StateCodes.USAGE_ERROR);
                _itemStatusMessage.state().streamState(StreamStates.CLOSED);
                _itemStatusMessage.state().text().data("Item request rejected for stream id " + msg.streamId() + "- QoS not supported");
                break;
            case ITEM_ALREADY_OPENED:
                _itemStatusMessage.state().code(StateCodes.ALREADY_OPEN);
                _itemStatusMessage.state().streamState(StreamStates.CLOSED);
                _itemStatusMessage.state().text().data("Item request rejected for stream id " + msg.streamId() + "- item already open with exact same key on another stream");
                break;
            case STREAM_ALREADY_IN_USE:
                _itemStatusMessage.state().code(StateCodes.USAGE_ERROR);
                _itemStatusMessage.state().streamState(StreamStates.CLOSED);
                _itemStatusMessage.state().text().data("Item request rejected for stream id " + msg.streamId() + "- stream already in use with a different key");
                break;
            case KEY_ENC_ATTRIB_NOT_SUPPORTED:
                _itemStatusMessage.state().code(StateCodes.USAGE_ERROR);
                _itemStatusMessage.state().streamState(StreamStates.CLOSED);
                _itemStatusMessage.state().text().data("Item request rejected for stream id " + msg.streamId() + "- this provider does not support key attribute information");
                break;
            case ITEM_NOT_SUPPORTED:
                _itemStatusMessage.state().code(StateCodes.USAGE_ERROR);
                _itemStatusMessage.state().streamState(StreamStates.CLOSED);
                _itemStatusMessage.state().text().data("Item request rejected for stream id " + msg.streamId() + "- item not supported");
                break;
            case PRIVATE_STREAM_REDIRECT:
                _itemStatusMessage.applyPrivateStream();
                _itemStatusMessage.state().code(StateCodes.NONE);
                _itemStatusMessage.state().streamState(StreamStates.REDIRECTED);
                _itemStatusMessage.state().text().data("Standard stream redirect to private for stream id " + msg.streamId() + " - this item must be requested via private stream");
                break;
            case PRIVATE_STREAM_MISMATCH:
                _itemStatusMessage.applyPrivateStream();
                _itemStatusMessage.state().code(StateCodes.USAGE_ERROR);
                _itemStatusMessage.state().streamState(StreamStates.CLOSED);
                _itemStatusMessage.state().text().data("Rejected request for stream id " + msg.streamId() + " - reissue via batch request is not allowed");
                break;
            default:
                break;
        }

        _eIter.clear();
        _itemStatusMessage.streamId(msg.streamId());
        _itemStatusMessage.domainType(msg.domainType());
        int ret = _eIter.setBufferAndRWFVersion(providerSession.writingBuffer(), providerSession.clientChannelInfo().channel.majorVersion(), providerSession.clientChannelInfo().channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            error.errorId(ret);
            return ret;
        }

        ret = _itemStatusMessage.encode(_eIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("MarketPriceStatus.encode() failed with return code: " + CodecReturnCodes.toString(ret));
            error.errorId(ret);
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /*
     * Is stream with stream id in use. 
     */
    private boolean isStreamInUse(ProviderSession providerSession, Integer streamId, MsgKey msgKey)
    {
        ItemInfo itemInfo = providerSession.itemAttributesTable().get(streamId);
        if (itemInfo == null)
            return false;

        return !itemInfo.attributes().msgKey().equals(msgKey);
    }

    /*
     * Retrieves item from the item watch list.
     */
    private ItemInfo findAlreadyOpenedItem(ProviderSession providerSession, Msg msg, ItemAttributes attributes)
    {
        return providerSession.itemAttributesTable().get(attributes);
    }

    /*
     * Decode post message received, sends updated market data message to the connected clients subscribed to the item. 
     */
    private int reflectPostMsg(ProviderThread providerThread, ProviderSession providerSession, DecodeIterator decodeIter, PostMsg postMsg, Error error)
    {
        int ret;
        switch (postMsg.containerType())
        {
            case DataTypes.MSG:
                _tmpMsg.clear();
                ret = _tmpMsg.decode(decodeIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return PerfToolsReturnCodes.FAILURE;
                break;
            default:
                // It's a container(e.g. field list). Add an update header for
                // reflecting.
                _tmpMsg.clear();
                _tmpMsg.msgClass(MsgClasses.UPDATE);
                ((UpdateMsg)_tmpMsg).containerType(postMsg.containerType());
                ((UpdateMsg)_tmpMsg).domainType(postMsg.domainType());
                postMsg.encodedDataBody().copy(_tmpMsg.encodedDataBody());
                break;

        }

        // get a buffer for the response
        ret = providerThread.getItemMsgBuffer(providerSession, 128 + _tmpMsg.encodedDataBody().length(), error);
        if (ret < PerfToolsReturnCodes.SUCCESS)
            return ret;

        // Add the post user info from the post message to the nested message
        // and re-encode.
        _eIter.clear();
        ret = _eIter.setBufferAndRWFVersion(providerSession.writingBuffer(), providerSession.clientChannelInfo().channel.majorVersion(), providerSession.clientChannelInfo().channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.err.println("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return PerfToolsReturnCodes.FAILURE;
        }

        // Add stream ID of PostMsg to nested message.
        _tmpMsg.streamId(postMsg.streamId());

        // Add PostUserInfo of PostMsg to nested message.
        switch (_tmpMsg.msgClass())
        {
            case MsgClasses.REFRESH:
                RefreshMsg tmpRefreshMsg = (RefreshMsg)_tmpMsg;
                tmpRefreshMsg.postUserInfo().userAddr(postMsg.postUserInfo().userAddr());
                tmpRefreshMsg.postUserInfo().userId(postMsg.postUserInfo().userId());
                tmpRefreshMsg.applyHasPostUserInfo();
                int flags = tmpRefreshMsg.flags();
                flags &= ~RefreshMsgFlags.SOLICITED;
                tmpRefreshMsg.flags(flags);
                break;
            case MsgClasses.UPDATE:
                UpdateMsg tmpUpdateMsg = (UpdateMsg)_tmpMsg;
                tmpUpdateMsg.postUserInfo().userAddr(postMsg.postUserInfo().userAddr());
                tmpUpdateMsg.postUserInfo().userId(postMsg.postUserInfo().userId());
                tmpUpdateMsg.applyHasPostUserInfo();
                break;
            case MsgClasses.STATUS:
                StatusMsg tmpStatusMsg = (StatusMsg)_tmpMsg;
                tmpStatusMsg.postUserInfo().userAddr(postMsg.postUserInfo().userAddr());
                tmpStatusMsg.postUserInfo().userId(postMsg.postUserInfo().userId());
                tmpStatusMsg.applyHasPostUserInfo();
                break;
            default:
                System.err.println("Error: Unhandled message class in post: " + MsgClasses.toString(_tmpMsg.msgClass()) + "(" + _tmpMsg.msgClass() + ")\n");
                return PerfToolsReturnCodes.FAILURE;
        }

        // Other header members & data body should be properly set, so
        // re-encode.
        if ((ret = _tmpMsg.encode(_eIter)) != CodecReturnCodes.SUCCESS)
            return PerfToolsReturnCodes.FAILURE;

        ret = providerThread.sendItemMsgBuffer(providerSession, true, error);
        if (ret < TransportReturnCodes.SUCCESS)
            return PerfToolsReturnCodes.FAILURE;

        return PerfToolsReturnCodes.SUCCESS;
    }

    int processGenMsg(ProviderThread providerThread, DecodeIterator decodeIter, ProviderSession providerSession, GenericMsg genMsg)
    {
        int ret;
        ret = decodePayload(providerThread, decodeIter, providerSession, genMsg);
        if (ret != CodecReturnCodes.SUCCESS)
            return PerfToolsReturnCodes.FAILURE;

        return PerfToolsReturnCodes.SUCCESS;
    }

    int decodePayload(ProviderThread providerThread, DecodeIterator decodeIter, ProviderSession providerSession, GenericMsg genMsg)
    {
		switch(genMsg.domainType())
		{
		case DomainTypes.MARKET_PRICE:
			_marketPriceDecoder.decodeUpdate(decodeIter, genMsg, providerThread.getProvThreadInfo());
			break;
		}
		return PerfToolsReturnCodes.SUCCESS;
    }
}