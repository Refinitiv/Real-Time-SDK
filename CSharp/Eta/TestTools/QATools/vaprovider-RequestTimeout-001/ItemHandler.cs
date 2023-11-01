/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.ValueAdd.Rdm;
using Array = LSEG.Eta.Codec.Array;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Provider
{
    /// <summary>
    /// This is the implementation of handling item requests for the interactive provider application. 
    /// It provides methods for processing item requests from consumers and sending back the refresh/update messages. 
    /// Methods for sending request reject/close status messages, initializing the item handler, checking if the item count 
    /// per channel has been reached, checking if an item is already opened on a channel, 
    /// checking if a stream is already in use, and closing item streams are also provided.
    /// This handler provides data for MarketPrice, MarketByPrice and MarketByOrder item requests.
    /// </summary>
    public class ItemHandler
    {
        private const int REJECT_MSG_SIZE = 1024;
        private const int ACK_MSG_SIZE = 1024;
        private const int ITEM_MSG_SIZE = 1024;
        private const int MAX_REFRESH_PARTS = 3;
        private const int POST_MSG_SIZE = 1024;
        private Buffer m_TriItemName = new Buffer();
        private Buffer m_PrivateStreamItemName = new Buffer();
        private Buffer m_slNameBuf = new Buffer();
        private Buffer m_batchReqName = new Buffer();

        private MarketPriceStatus m_MarketPriceStatus;
        private SymbolListItems m_SymbolListItemWatchList;
        private MarketByOrderItems m_MarketByOrderItemWatchList;
        private MarketByPriceItems m_MarketByPriceItemWatchList;
        private MarketPriceItems m_MarketPriceItemWatchList;
        private ItemInfoList m_ItemInfoWatchList;
        private ItemRequestInfoList m_ItemRequestWatchList;

        private DictionaryHandler m_DictionaryHandler;
        private LoginHandler m_LoginHandler;

        private int m_ItemCount = 0;

        private WriteArgs m_WriteArgs = new WriteArgs();
        private Error m_Error = new Error();
        private ElementList m_ElementList = new ElementList();
        private ElementEntry m_ElementEntry = new ElementEntry();
        private Array m_Array = new Array();
        private ArrayEntry m_ArrayEntry = new ArrayEntry();
        private EncodeIterator m_EncodeIter = new EncodeIterator();
        private IAckMsg m_AckMsg = new Msg();
        private Msg m_NestedMsg = new Msg();
        private IUpdateMsg m_UpdateMsg = new Msg();
        private IStatusMsg m_StatusMsg = new Msg();
        private Qos m_ProviderQos = new Qos();

        private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();

        public int ServiceId { get; set; }

        public ItemHandler(DictionaryHandler dictionaryHandler, LoginHandler loginHandler)
        {
            m_MarketByOrderItemWatchList = new MarketByOrderItems();
            m_MarketByPriceItemWatchList = new MarketByPriceItems();
            m_MarketPriceItemWatchList = new MarketPriceItems();
            m_MarketPriceStatus = new MarketPriceStatus();
            m_SymbolListItemWatchList = new SymbolListItems();
            m_ItemInfoWatchList = new ItemInfoList();
            m_ItemRequestWatchList = new ItemRequestInfoList();

            m_DictionaryHandler = dictionaryHandler;
            m_LoginHandler = loginHandler;

            m_UpdateMsg.MsgClass = MsgClasses.UPDATE;
            m_AckMsg.MsgClass = MsgClasses.ACK;
            m_StatusMsg.MsgClass = MsgClasses.STATUS;

            m_TriItemName.Data("TRI");
            m_PrivateStreamItemName.Data("RES-DS");
            m_slNameBuf.Data("_ETA_ITEM_LIST");
            m_batchReqName.Data(":ItemList");

            m_ProviderQos.IsDynamic = false;
            m_ProviderQos.Rate(QosRates.TICK_BY_TICK);
            m_ProviderQos.Timeliness(QosTimeliness.REALTIME);
        }

        /// <summary>
        /// Initializes item handler internal structures.
        /// </summary>
        public void Init()
        {
            m_ItemRequestWatchList.Init();
            m_ItemInfoWatchList.Init();
            m_MarketByOrderItemWatchList.Init();
            m_MarketByPriceItemWatchList.Init();
            m_MarketPriceItemWatchList.Init();
            m_SymbolListItemWatchList.Init();
        }

        /// <summary>
        /// Processes an item request. This consists of storing the request information, 
        /// then calling sendItemResponse() to send the response.
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> that corresponds to the current client</param>
        /// <param name="msg">the request message</param>
        /// <param name="dIter">the <see cref="DecodeIterator"/> instance used to decode current message</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> structure that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value that indicates the status of the operation</returns>
        public ReactorReturnCode ProcessRequest(ReactorChannel chnl, Msg msg, DecodeIterator dIter, out ReactorErrorInfo? errorInfo)
        {
            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    if (!msg.MsgKey.CheckHasServiceId() || msg.MsgKey.ServiceId != ServiceId)
                    {
                        return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.INVALID_SERVICE_ID, false, out errorInfo);
                    }

                    //check if QoS supported 
                    if ((msg.Flags & (int)RequestMsgFlags.HAS_WORST_QOS) != 0 && (msg.Flags & (int)RequestMsgFlags.HAS_QOS) != 0)
                    {
                        if (!msg.Qos.IsInRange(m_ProviderQos, msg.WorstQos))
                        {
                            return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.QOS_NOT_SUPPORTED, false, out errorInfo);
                        }
                    }
                    else if ((msg.Flags & (int)RequestMsgFlags.HAS_QOS) != 0)
                    {
                        if (!msg.Qos.Equals(m_ProviderQos))
                        {
                            return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.QOS_NOT_SUPPORTED, false, out errorInfo);
                        }
                    }

                    //check for unsupported key attribute information
                    if (msg.MsgKey.CheckHasAttrib())
                    {
                        return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.KEY_ENC_ATTRIB_NOT_SUPPORTED, false, out errorInfo);
                    }

                    if (msg.CheckHasBatch())
                    {
                        return ProcessBatchRequest(chnl, msg, dIter, msg.CheckPrivateStream(), out errorInfo);
                    }
                    else
                    {
                        return ProcessSingleItemRequest(chnl, msg, dIter, msg.CheckPrivateStream(), out errorInfo);
                    }
                case MsgClasses.CLOSE:
                    Console.WriteLine($"Received Item close for streamId {msg.StreamId}");
                    CloseStream(chnl.Channel!, msg.StreamId);
                    break;
                case MsgClasses.POST:
                    return ProcessPost(chnl, msg, dIter, out errorInfo);
                default:
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = $"Received Unhandled Item Msg Class: {MsgClasses.ToString(msg.MsgClass)}";
                    return ReactorReturnCode.FAILURE;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends item request reject message
        /// </summary>
        /// <param name="channel">the <see cref="ReactorChannel"/> that corresponds to the current client</param>
        /// <param name="streamId">the id if the item stream</param>
        /// <param name="domainType">the item request domain type</param>
        /// <param name="reason">the reject reason</param>
        /// <param name="isPrivateStream">indicates whether a private stream was requested</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> structure that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value that indicates the status of the operation</returns>
        public ReactorReturnCode SendItemRequestReject(ReactorChannel channel, int streamId, int domainType, ItemRejectReason reason, bool isPrivateStream, out ReactorErrorInfo? errorInfo)
        {
            //get a buffer for the item request reject status
            ITransportBuffer? msgBuf = channel.GetBuffer(REJECT_MSG_SIZE, false, out errorInfo);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            m_MarketPriceStatus.Clear();
            m_MarketPriceStatus.HasState = true;
            m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
            m_MarketPriceStatus.State.DataState(DataStates.SUSPECT);
            if (isPrivateStream)
                m_MarketPriceStatus.PrivateStream = true;

            //encode request reject status
            switch (reason)
            {
                case ItemRejectReason.ITEM_COUNT_REACHED:
                    m_MarketPriceStatus.State.Code(StateCodes.TOO_MANY_ITEMS);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId} - item count reached for this channel");
                    break;
                case ItemRejectReason.INVALID_SERVICE_ID:
                    m_MarketPriceStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId} - service id invalid");
                    break;
                case ItemRejectReason.QOS_NOT_SUPPORTED:
                    m_MarketPriceStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId} - QoS not supported");
                    break;
                case ItemRejectReason.ITEM_ALREADY_OPENED:
                    m_MarketPriceStatus.State.Code(StateCodes.ALREADY_OPEN);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId} - item already open with exact same key on another stream");
                    break;
                case ItemRejectReason.STREAM_ALREADY_IN_USE:
                    m_MarketPriceStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId} - stream already in use with a different key");
                    break;
                case ItemRejectReason.KEY_ENC_ATTRIB_NOT_SUPPORTED:
                    m_MarketPriceStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId} - this provider does not support key attribute information");
                    break;
                case ItemRejectReason.ITEM_NOT_SUPPORTED:
                    m_MarketPriceStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId} - item not supported");
                    break;
                case ItemRejectReason.PRIVATE_STREAM_REDIRECT:
                    m_MarketPriceStatus.PrivateStream = true;
                    m_MarketPriceStatus.State.Code(StateCodes.NONE);
                    m_MarketPriceStatus.State.StreamState(StreamStates.REDIRECTED);
                    m_MarketPriceStatus.State.Text().Data($"Standard stream redirect to private for stream id {streamId} - this item must be requested via private stream");
                    break;
                case ItemRejectReason.PRIVATE_STREAM_MISMATCH:
                    m_MarketPriceStatus.PrivateStream = true;
                    m_MarketPriceStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Rejected request for stream id {streamId} - reissue via batch request is not allowed");
                    break;
                case ItemRejectReason.DOMAIN_NOT_SUPPORTED:
                    m_MarketPriceStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId} - domain type {domainType} is not supported");
                    break;
                default:
                    break;
            }
            m_EncodeIter.Clear();
            m_MarketPriceStatus.StreamId = streamId;
            m_MarketPriceStatus.DomainType = domainType;
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                if (errorInfo is null) errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            ret = m_MarketPriceStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                if (errorInfo is null) errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"MarketPriceStatus.Encode() failed: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            Console.WriteLine($"Rejecting Item Request with streamId = {streamId} and domain {DomainTypes.ToString(domainType)}. Reason: {reason.GetAsString()}");

            return channel.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

        /// <summary>
        /// Sends the item close status message(s) for a channel. 
        /// This consists of finding all request information for this channel 
        /// and sending the close status messages to the channel.
        /// </summary>
        /// <param name="channel">the <see cref="ReactorChannel"/> associated with the current client</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> structure that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value that indicates the status of the operation</returns>
        public ReactorReturnCode SendCloseStatusMsgs(ReactorChannel channel, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;
            foreach (ItemRequestInfo itemRequestInfo in m_ItemRequestWatchList)
            {
                if (itemRequestInfo.IsInUse && itemRequestInfo.Channel == channel.Channel)
                {
                    ret = SendCloseStatus(channel, itemRequestInfo, out errorInfo);
                    if (ret != ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends the item update(s) for a channel. 
        /// This consists of finding all request information for this channel, 
        /// and sending the responses to the channel.
        /// </summary>
        /// <param name="channel">the <see cref="ReactorChannel"/> associated with the current client</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> structure that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value that indicates the status of the operation</returns>
        public ReactorReturnCode SendItemUpdates(ReactorChannel channel, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;
            foreach (ItemRequestInfo itemReqInfo in m_ItemRequestWatchList)
            {
                if (itemReqInfo.IsInUse && itemReqInfo.Channel == channel.Channel)
                {
                    ret = SendItemResponse(channel, itemReqInfo, out errorInfo);
                    if (ret != ReactorReturnCode.SUCCESS)
                        return ret;
                }
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode SendItemResponse(ReactorChannel chnl, ItemRequestInfo itemReqInfo, out ReactorErrorInfo? errorInfo)
        {
            //market by price is handled separately due to multi-part refresh
            if (itemReqInfo.DomainType == (int)DomainType.MARKET_BY_PRICE)
            {
                //APIQA: Comment sending response and return success
                //return SendMBPItemResponse(chnl, itemReqInfo, out errorInfo);
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
                //END APIQA
            }

            //get a buffer for the response
            ITransportBuffer? msgBuf = chnl.GetBuffer(ITEM_MSG_SIZE, false, out errorInfo);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            Error? error;
            //Encode the message with data appropriate for the domain
            switch (itemReqInfo.DomainType)
            {
                case (int)DomainType.MARKET_PRICE:
                    //encode market price response 
                    CodecReturnCode ret = m_MarketPriceItemWatchList.EncodeResponse(chnl.Channel!, itemReqInfo.ItemInfo!, msgBuf, true, itemReqInfo.StreamId,
                                                                                   itemReqInfo.IsStreamingRequest, itemReqInfo.IsPrivateStreamRequest, 
                                                                                   ServiceId, m_DictionaryHandler.Dictionary, out error);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        errorInfo = new ReactorErrorInfo();
                        errorInfo.Error.Text = error != null ? error.Text : $"Failed to encode Market Price item response: {ret.GetAsString()}";
                        return ReactorReturnCode.FAILURE;
                    }
                    break;
                case (int)DomainType.MARKET_BY_ORDER:
                    //encode market by order response
                    ret = m_MarketByOrderItemWatchList.EncodeResponse(chnl.Channel!, itemReqInfo.ItemInfo!, msgBuf, true, itemReqInfo.StreamId, 
                                                                     itemReqInfo.IsStreamingRequest, itemReqInfo.IsPrivateStreamRequest, 
                                                                     ServiceId, m_DictionaryHandler.Dictionary, out error);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        errorInfo = new ReactorErrorInfo();
                        errorInfo.Error.Text = error != null ? error.Text : $"Failed to encode Market By Order item response: {ret.GetAsString()}";
                        return ReactorReturnCode.FAILURE;
                    }
                    break;
                case (int)DomainType.SYMBOL_LIST:
                    //encode symbol list response
                    //only encode refresh responses for the symbol list from this
                    //method. symbol list update responses are handled separately
                    if (itemReqInfo.ItemInfo!.IsRefreshRequired)
                    {
                        ret = m_SymbolListItemWatchList.EncodeResponse(itemReqInfo.Channel!, itemReqInfo.ItemInfo, msgBuf, itemReqInfo.StreamId, true,
                                                                      ServiceId, itemReqInfo.IsStreamingRequest, m_DictionaryHandler.Dictionary,
                                                                      SymbolListItems.SYMBOL_LIST_REFRESH, out error);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            errorInfo = new ReactorErrorInfo();
                            errorInfo.Error.Text = error != null ? error.Text : $"Failed to encode Symbol List response: {ret.GetAsString()}";
                            return ReactorReturnCode.FAILURE;
                        }
                    }
                    else
                    {
                        chnl.ReleaseBuffer(msgBuf, out errorInfo);
                        return ReactorReturnCode.SUCCESS;
                    }
                    break;
                default:
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = $"Received unhandled domain {itemReqInfo.DomainType} for item with stream id {itemReqInfo.StreamId}";
                    return ReactorReturnCode.FAILURE;
            }
            //send item response
            //APIQA: Comment sending response and return success
            //return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
            //END APIQA
        }

        private ReactorReturnCode SendMBPItemResponse(ReactorChannel chnl, ItemRequestInfo itemReqInfo, out ReactorErrorInfo? errorInfo)
        {
            MarketByPriceItem mbpItem;

            if (itemReqInfo.ItemInfo!.IsRefreshRequired)
            {
                mbpItem = (MarketByPriceItem)itemReqInfo.ItemInfo.ItemData!;
                return SendMBPItemRefreshResponse(chnl, mbpItem, itemReqInfo, out errorInfo);             
            }
            else
            {
                //get a buffer for the response
                ITransportBuffer? msgBuf = chnl.GetBuffer(ITEM_MSG_SIZE, false, out errorInfo);
                if (msgBuf == null)
                {
                    return ReactorReturnCode.FAILURE;
                }

                //encode market by price update
                CodecReturnCode codecReturnCode = m_MarketByPriceItemWatchList.EncodeUpdate(chnl.Channel!, itemReqInfo.ItemInfo, msgBuf, true, itemReqInfo.StreamId,
                                                                                           itemReqInfo.IsStreamingRequest, itemReqInfo.IsPrivateStreamRequest, 
                                                                                           ServiceId, m_DictionaryHandler.Dictionary, out Error? error);
                
                if (codecReturnCode != CodecReturnCode.SUCCESS)
                {
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = error != null ? error.Text : $"Failed encoding update: {codecReturnCode.GetAsString()}";
                    return ReactorReturnCode.FAILURE;
                }

                //send item response
                return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
            }
        }

        public ReactorReturnCode SendMBPItemRefreshResponse(ReactorChannel chnl, MarketByPriceItem mbpItem, ItemRequestInfo itemReqInfo, out ReactorErrorInfo? errorInfo)
        {
            CodecReturnCode codecReturnCode;
            ReactorReturnCode reactorReturnCode;

            for (int i = 0; i < MAX_REFRESH_PARTS; i++)
            {
                //get a buffer for the response
                ITransportBuffer? msgBuf = chnl.GetBuffer(ITEM_MSG_SIZE, false, out errorInfo);
                if (msgBuf == null)
                    return ReactorReturnCode.FAILURE;

                if (msgBuf != null)
                {
                    //encode market by price refresh 
                    codecReturnCode = m_MarketByPriceItemWatchList.EncodeRefresh(chnl.Channel!, itemReqInfo.ItemInfo!, msgBuf, true, itemReqInfo.StreamId,
                                                                               itemReqInfo.IsStreamingRequest, itemReqInfo.IsPrivateStreamRequest,
                                                                               ServiceId, m_DictionaryHandler.Dictionary, i, out Error? error);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        errorInfo = new ReactorErrorInfo();
                        errorInfo.Error.Text = error != null ? error.Text : $"Failed to encode refresh: {codecReturnCode.GetAsString()}";
                        return ReactorReturnCode.FAILURE;
                    }

                    //send item response
                    reactorReturnCode = chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
                    if (reactorReturnCode != ReactorReturnCode.SUCCESS)
                    {
                        return reactorReturnCode;
                    }
                }

                //send an update between each part of the refresh
                if (i < MAX_REFRESH_PARTS - 1)
                {
                    mbpItem.PriceInfoList[0].ORDER_SIZE.Value(mbpItem.PriceInfoList[0].ORDER_SIZE.ToDouble() + i + 1, mbpItem.PriceInfoList[0].ORDER_SIZE.Hint);
                    mbpItem.PriceInfoList[1].ORDER_SIZE.Value(mbpItem.PriceInfoList[1].ORDER_SIZE.ToDouble() + i + 1, mbpItem.PriceInfoList[1].ORDER_SIZE.Hint); 
                    mbpItem.PriceInfoList[2].ORDER_SIZE.Value(mbpItem.PriceInfoList[2].ORDER_SIZE.ToDouble() + i + 1, mbpItem.PriceInfoList[2].ORDER_SIZE.Hint);
                    msgBuf = chnl.GetBuffer(ITEM_MSG_SIZE, false, out errorInfo);
                    if (msgBuf == null)
                    {
                        return ReactorReturnCode.FAILURE;
                    }
                    //encode market by price update
                    codecReturnCode = m_MarketByPriceItemWatchList.EncodeUpdate(chnl.Channel!, itemReqInfo.ItemInfo!, msgBuf, true, itemReqInfo.StreamId,
                                                                               itemReqInfo.IsStreamingRequest, itemReqInfo.IsPrivateStreamRequest,
                                                                               ServiceId, m_DictionaryHandler.Dictionary, out Error? error);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        errorInfo = new ReactorErrorInfo();
                        errorInfo.Error.Text = error != null ? error.Text : $"Failed to encode item update: {codecReturnCode.GetAsString()}";
                        return ReactorReturnCode.FAILURE;
                    }

                    //send item response
                    reactorReturnCode = chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
                    if (reactorReturnCode != ReactorReturnCode.SUCCESS)
                    {
                        return reactorReturnCode;
                    }

                    mbpItem.PriceInfoList[0].ORDER_SIZE.Value(mbpItem.PriceInfoList[0].ORDER_SIZE.ToDouble() - (i + 1), mbpItem.PriceInfoList[0].ORDER_SIZE.Hint);
                    mbpItem.PriceInfoList[1].ORDER_SIZE.Value(mbpItem.PriceInfoList[1].ORDER_SIZE.ToDouble() - (i + 1), mbpItem.PriceInfoList[1].ORDER_SIZE.Hint); 
                    mbpItem.PriceInfoList[2].ORDER_SIZE.Value(mbpItem.PriceInfoList[2].ORDER_SIZE.ToDouble() - (i + 1), mbpItem.PriceInfoList[2].ORDER_SIZE.Hint); 
                }
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private CodecReturnCode CopyMsgKey(IMsgKey destKey, IMsgKey sourceKey)
        {
            destKey.Flags = sourceKey.Flags;
            destKey.NameType = sourceKey.NameType;
            if (sourceKey.CheckHasName() && sourceKey.Name != null)
            {
                BufferHelper.CopyBuffer(sourceKey.Name, destKey.Name);
            }
            destKey.ServiceId = sourceKey.ServiceId;
            destKey.Filter = sourceKey.Filter;
            destKey.Identifier = sourceKey.Identifier;
            destKey.AttribContainerType = sourceKey.AttribContainerType;
            if (sourceKey.CheckHasAttrib() && sourceKey.EncodedAttrib != null)
            {
                return sourceKey.EncodedAttrib.Copy(destKey.EncodedAttrib);                 
            }

            return CodecReturnCode.SUCCESS;
        }

        private bool IsStreamInUse(ReactorChannel chnl, int streamId, IMsgKey key)
        {
            bool streamInUse = false;

            foreach (ItemRequestInfo itemReqInfo in m_ItemRequestWatchList)
            {
                if (itemReqInfo.IsInUse &&
                        itemReqInfo.Channel == chnl.Channel &&
                        itemReqInfo.StreamId == streamId)
                {
                    if (itemReqInfo.MsgKey.Equals(key))
                    {
                        streamInUse = true;
                        break;
                    }
                }
            }

            return streamInUse;
        }

        private ReactorReturnCode ProcessPost(ReactorChannel chnl, Msg msg, DecodeIterator dIter, out ReactorErrorInfo? errorInfo)
        {
            LoginRequestInfo? loginRequestInfo;
            IPostMsg postMsg = (IPostMsg)msg;
            ReactorReturnCode reactorReturnCode;
            CodecReturnCode codecReturnCode;

            // get the login stream so that we can see if the post was an off-stream
            // post
            if ((loginRequestInfo = m_LoginHandler.FindLoginRequestInfo(chnl.Channel!)) == null)
            {
                return SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, "Received a post message request from client before login", out errorInfo);
            }

            ItemInfo? itemInfo;
            // if the post is on the login stream, then it's an off-stream post
            if (loginRequestInfo.LoginRequest.StreamId == msg.StreamId)
            {
                // the msg key must be specified to provide the item name
                if (!postMsg.CheckHasMsgKey())
                {
                    return SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, "Received an off-stream post message request from client without a msgkey", out errorInfo);
                }
                Console.WriteLine($"Received an off-stream item post (item={postMsg.MsgKey.Name})");
                // look up the item name for this example, we will treat an unknown item as an error
                // However, other providers may choose to add the item to their cache
                if ((itemInfo = m_ItemInfoWatchList.Get(postMsg.MsgKey.Name, postMsg.DomainType, false)) == null)
                {
                    return SendAck(chnl, postMsg, NakCodes.SYMBOL_UNKNOWN, "Received an off-stream post message for an unknown item", out errorInfo);
                }
            }
            else
            {
                ItemRequestInfo? itemReqInfo = null;
                // the msgkey is not required for on-stream post
                // get the item request associated with this on-stream post
                if ((itemReqInfo = m_ItemRequestWatchList.Get(chnl.Channel!, postMsg.StreamId)) == null)
                {
                    return SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, "Received an on-stream post message on a stream that does not have an item open", out errorInfo);
                }

                itemInfo = itemReqInfo.ItemInfo;
                Console.WriteLine($"Received an on-stream post for item={itemInfo!.ItemName}");
            }


            // if the post message contains another message, then use the
            // "contained" message as the update/refresh/status
            if (postMsg.ContainerType == DataTypes.MSG)
            {
                m_NestedMsg.Clear();
                codecReturnCode = m_NestedMsg.Decode(dIter);
                if (codecReturnCode != CodecReturnCode.SUCCESS)
                {
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = $"Unable to decode msg: {codecReturnCode.GetAsString()}";
                    return ReactorReturnCode.FAILURE;
                }

                switch (m_NestedMsg.MsgClass)
                {
                    case MsgClasses.REFRESH:
                        m_NestedMsg.MsgClass = MsgClasses.REFRESH;
                        m_NestedMsg.Flags = (m_NestedMsg.Flags | RefreshMsgFlags.HAS_POST_USER_INFO) & ~RefreshMsgFlags.SOLICITED;

                        m_NestedMsg.PostUserInfo.UserAddr = postMsg.PostUserInfo.UserAddr;
                        m_NestedMsg.PostUserInfo.UserId = postMsg.PostUserInfo.UserId;
                        if (UpdateItemInfoFromPost(itemInfo, m_NestedMsg, dIter, out errorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            reactorReturnCode = SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, errorInfo != null ? errorInfo.Error.Text : "", out errorInfo);
                            if (reactorReturnCode != ReactorReturnCode.SUCCESS)
                            {
                                return reactorReturnCode;
                            }
                        }

                        break;

                    case MsgClasses.UPDATE:
                        m_NestedMsg.MsgClass = MsgClasses.UPDATE;
                        m_NestedMsg.Flags = m_NestedMsg.Flags | UpdateMsgFlags.HAS_POST_USER_INFO;
                        m_NestedMsg.PostUserInfo.UserAddr = postMsg.PostUserInfo.UserAddr;
                        m_NestedMsg.PostUserInfo.UserId = postMsg.PostUserInfo.UserId;
                        if (UpdateItemInfoFromPost(itemInfo, m_NestedMsg, dIter, out errorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            reactorReturnCode = SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, errorInfo != null ? errorInfo.Error.Text : "", out errorInfo);
                            if (reactorReturnCode != ReactorReturnCode.SUCCESS)
                            {
                                return reactorReturnCode;
                            }
                        }
                        break;

                    case MsgClasses.STATUS:
                        m_NestedMsg.MsgClass = MsgClasses.STATUS;
                        m_NestedMsg.Flags = m_NestedMsg.Flags | StatusMsgFlags.HAS_POST_USER_INFO;
                        m_NestedMsg.PostUserInfo.UserAddr = postMsg.PostUserInfo.UserAddr;
                        m_NestedMsg.PostUserInfo.UserId = postMsg.PostUserInfo.UserId;
                        if (m_NestedMsg.CheckHasState() && (m_NestedMsg.State.StreamState() == StreamStates.CLOSED))
                        {
                            // check if the user has the rights to send a post that closes an item
                            if (postMsg.CheckHasPostUserRights() || postMsg.PostUserRights == 0)
                            {
                                reactorReturnCode = SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, "client has insufficient rights to close/delete an item", out errorInfo);
                                if (reactorReturnCode != ReactorReturnCode.SUCCESS)
                                    return reactorReturnCode;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            else
            {
                // It's a container (e.g. field list). Add an update header for reflecting.
                m_UpdateMsg.Clear();
                m_UpdateMsg.MsgClass = MsgClasses.UPDATE;
                m_UpdateMsg.DomainType = postMsg.DomainType;
                m_UpdateMsg.ContainerType = postMsg.ContainerType;
                if (msg.EncodedDataBody != null && msg.EncodedDataBody.Length > 0)
                {
                    m_UpdateMsg.EncodedDataBody = msg.EncodedDataBody;
                }
                m_UpdateMsg.Flags = UpdateMsgFlags.HAS_POST_USER_INFO;
                m_UpdateMsg.PostUserInfo.UserAddr = postMsg.PostUserInfo.UserAddr;
                m_UpdateMsg.PostUserInfo.UserId = postMsg.PostUserInfo.UserId;
                if (postMsg.CheckHasMsgKey())
                {
                    m_UpdateMsg.Flags = m_UpdateMsg.Flags | UpdateMsgFlags.HAS_MSG_KEY;
                    m_UpdateMsg.MsgKey.Copy(postMsg.MsgKey);
                }

                if (UpdateItemInfoFromPost(itemInfo, msg, dIter, out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    reactorReturnCode = SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, errorInfo != null ? errorInfo.Error.Text : "", out errorInfo);
                    if (reactorReturnCode != ReactorReturnCode.SUCCESS)
                    {
                        return reactorReturnCode;
                    }
                }
            }

            reactorReturnCode = SendAck(chnl, postMsg, NakCodes.NONE, "", out errorInfo);
            if (reactorReturnCode != ReactorReturnCode.SUCCESS)
            {
                return reactorReturnCode;
            }

            // send the post to all public streams with this item open
            foreach (ItemRequestInfo itemReqInfoL in m_ItemRequestWatchList)
            {
                if (itemReqInfoL.ItemInfo == itemInfo)
                {
                    m_EncodeIter.Clear();
                    ITransportBuffer? sendBuf = itemReqInfoL.Channel!.GetBuffer(POST_MSG_SIZE, false, out Error error);
                    if (sendBuf == null)
                    {
                        errorInfo = new ReactorErrorInfo();
                        errorInfo.Error.Text = error.Text;
                        return ReactorReturnCode.FAILURE;
                    }
                    codecReturnCode = m_EncodeIter.SetBufferAndRWFVersion(sendBuf, itemReqInfoL.Channel.MajorVersion, itemReqInfoL.Channel.MinorVersion);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        errorInfo = new ReactorErrorInfo();
                        errorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {codecReturnCode.GetAsString()}";
                        return ReactorReturnCode.FAILURE;
                    }

                    if (postMsg.ContainerType == DataTypes.MSG)
                    {
                        // send the contained/embedded message if there was one.
                        m_NestedMsg.StreamId = itemReqInfoL.StreamId;
                        if (m_NestedMsg.MsgClass == MsgClasses.REFRESH)
                        {
                            m_NestedMsg.ApplyHasMsgKey();
                        }
                        codecReturnCode = m_NestedMsg.Encode(m_EncodeIter);
                        if (codecReturnCode != CodecReturnCode.SUCCESS)
                        {
                            errorInfo = new ReactorErrorInfo();
                            errorInfo.Error.Text = $"nestedMsg.Encode() failed with return code: {codecReturnCode}";
                            return ReactorReturnCode.FAILURE;
                        }

                        reactorReturnCode = itemReqInfoL.ReactorChannel!.Submit(sendBuf, m_SubmitOptions, out errorInfo);
                        if (reactorReturnCode < ReactorReturnCode.SUCCESS)
                        {
                            return ReactorReturnCode.FAILURE;
                        }

                        // check if its a status close and close any open streams if it is
                        if (m_NestedMsg.MsgClass == MsgClasses.STATUS && m_NestedMsg.CheckHasState() && m_NestedMsg.State.StreamState() == StreamStates.CLOSED)
                        {
                            CloseStream(itemReqInfoL.Channel, m_NestedMsg.StreamId);
                        }
                    }
                    else
                    {
                        // send an update message if the post contained data
                        m_UpdateMsg.StreamId = itemReqInfoL.StreamId;
                        codecReturnCode = m_UpdateMsg.Encode(m_EncodeIter);
                        if (codecReturnCode != CodecReturnCode.SUCCESS)
                        {
                            errorInfo = new ReactorErrorInfo();
                            errorInfo.Error.Text = $"nestedMsg.Encode() failed with return code: {codecReturnCode.GetAsString()}";
                            return ReactorReturnCode.FAILURE;
                        }

                        reactorReturnCode = itemReqInfoL.ReactorChannel!.Submit(sendBuf, m_SubmitOptions, out errorInfo);
                        if (reactorReturnCode < ReactorReturnCode.SUCCESS)
                            return ReactorReturnCode.FAILURE;
                    }
                }
            }

            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode UpdateItemInfoFromPost(ItemInfo itemInfo, Msg msg, DecodeIterator dIter, out ReactorErrorInfo? errorInfo)
        {
            switch (itemInfo.DomainType)
            {
                case (int)DomainType.MARKET_PRICE:
                    CodecReturnCode codecReturnCode = m_MarketPriceItemWatchList.UpdateFieldsFromPost((MarketPriceItem)itemInfo.ItemData!, dIter, out Error? error);
                    if (codecReturnCode < CodecReturnCode.SUCCESS)
                    {
                        errorInfo = new ReactorErrorInfo();
                        errorInfo.Error.Text = error != null ? error.Text : $"Failed to update fields from post: {codecReturnCode.GetAsString()}";
                        return ReactorReturnCode.FAILURE;
                    }
                    break;
                case (int)DomainType.MARKET_BY_ORDER:
                case (int)DomainType.MARKET_BY_PRICE:
                default:
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = $"Unsupported domain {itemInfo.DomainType} in post message update/refresh";
                    return ReactorReturnCode.FAILURE;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public void CloseStream(ReactorChannel reactorChannel)
        {
            //find original item request information associated with channel
            foreach (ItemRequestInfo itemRequestInfoL in m_ItemRequestWatchList)
            {
                if (itemRequestInfoL.Channel == reactorChannel.Channel && itemRequestInfoL.IsInUse)
                {
                    FreeItemReqInfo(itemRequestInfoL, out ReactorErrorInfo? errorInfo);
                    if (errorInfo != null)
                    {
                        Console.WriteLine($"Error in FreeItemReqInfo with streamId = {itemRequestInfoL.StreamId}: {errorInfo.Error.Text}");
                    }
                }
            }
        }

        public void UpdateItemInfo()
        {
            m_ItemInfoWatchList.Update();
        }

        private ReactorReturnCode ProcessBatchRequest(ReactorChannel chnl, Msg msg, DecodeIterator dIter, bool isPrivateStream, out ReactorErrorInfo? errorInfo)
        {
            Console.WriteLine($"Received batch item request (streamId={msg.StreamId}) on domain {msg.DomainType}");

            // Check if batch stream already in use with a different key
            if (IsStreamInUse(chnl, msg.StreamId, msg.MsgKey))
            {
                return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.STREAM_ALREADY_IN_USE, isPrivateStream, out errorInfo);
            }

            // The payload of a batch request contains an elementList
            CodecReturnCode ret = m_ElementList.Decode(dIter, null);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.REQUEST_DECODE_ERROR, isPrivateStream, out errorInfo);
            }

            HashSet<ItemRejectReason> rejectReasonSet = new HashSet<ItemRejectReason>();
            int dataState = DataStates.NO_CHANGE;
            // The list of items being requested is in an elementList entry with the
            // element name of ":ItemList"
            while ((ret = m_ElementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (m_ElementEntry.Name.Equals(m_batchReqName))
                {
                    // The list of items names is in an array
                    ret = m_Array.Decode(dIter);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        errorInfo = new ReactorErrorInfo();
                        errorInfo.Error.Text = $"Array.Decode() for batch request failed with return code: {ret.GetAsString()}";
                        break;
                    }

                    // Get each requested item name
                    // We will assign consecutive stream IDs for each item
                    // starting with the stream following the one the batch request
                    // was made on
                    int itemStream = msg.StreamId;

                    while ((ret = m_ArrayEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                    {
                        if (ret < CodecReturnCode.SUCCESS)
                        {
                            errorInfo = new ReactorErrorInfo();
                            errorInfo.Error.Text = $"ArrayEntry.Decode() failed with return code: {ret.GetAsString()}";
                            dataState = DataStates.SUSPECT;
                            continue;
                        }

                        //check if stream already in use with a different key
                        itemStream++; // increment stream ID with each item we find in the batch request


                        //check for private stream special item name without
                        //private stream flag set
                        if (!isPrivateStream && m_ArrayEntry.EncodedData.Equals(m_PrivateStreamItemName))
                        {
                            SendItemRequestReject(chnl, itemStream, msg.DomainType, ItemRejectReason.PRIVATE_STREAM_REDIRECT, isPrivateStream, out errorInfo);
                            dataState = DataStates.SUSPECT;
                            continue;
                        }

                        //all of the items requested have the same key. They use
                        //the key of the batch request.
                        //The only difference is the name
                        msg.MsgKey.Flags = msg.MsgKey.Flags | MsgKeyFlags.HAS_NAME;
                        msg.MsgKey.Name = m_ArrayEntry.EncodedData;

                        ItemRequestInfo? itemReqInfo = GetMatchingItemReqInfo(chnl.Channel!, msg, itemStream, rejectReasonSet);
                        ItemRejectReason rejectReason = rejectReasonSet.Count > 0 ? rejectReasonSet.ElementAt(0) : ItemRejectReason.NONE;
                        if (itemReqInfo == null && rejectReason != ItemRejectReason.NONE)
                        {
                            SendItemRequestReject(chnl, itemStream, msg.DomainType, rejectReason, isPrivateStream, out errorInfo);
                            dataState = DataStates.SUSPECT;
                            continue;
                        }
                        else if (itemReqInfo != null)
                        {
                            // Batch requests should not be used to reissue item
                            // requests.
                            SendItemRequestReject(chnl, itemStream, msg.DomainType, ItemRejectReason.BATCH_ITEM_REISSUE, isPrivateStream, out errorInfo);
                            dataState = DataStates.SUSPECT;
                            continue;
                        }

                        rejectReasonSet.Clear();
                        itemReqInfo = GetNewItemReqInfo(chnl, msg, itemStream, rejectReasonSet, out errorInfo);
                        if (itemReqInfo == null)
                        {
                            // Batch requests should not be used to reissue item requests.
                            SendItemRequestReject(chnl, itemStream, msg.DomainType, rejectReasonSet.ElementAt(0), isPrivateStream, out errorInfo);
                            dataState = DataStates.SUSPECT;
                            continue;
                        }

                        if (msg.CheckPrivateStream())
                        {
                            Console.WriteLine($"Received Private Stream Item Request for {itemReqInfo.ItemName}" +
                                               $"(streamId={itemStream}) on domain {DomainTypes.ToString(itemReqInfo.DomainType)}");
                        }
                        else
                        {
                            Console.WriteLine($"Received Item Request for {itemReqInfo.ItemName}" +
                                               $"(streamId={itemStream}) on domain {DomainTypes.ToString(itemReqInfo.DomainType)}");
                        }

                        //send item response/refresh if required
                        if (!msg.CheckNoRefresh())
                        {
                            SendItemResponse(chnl, itemReqInfo, out errorInfo);
                        }

                        if (!itemReqInfo.IsStreamingRequest)
                        {
                            //snapshot request - so we dont have to send updates
                            //free item request info
                            FreeItemReqInfo(itemReqInfo, out errorInfo);
                        }
                        else
                        {
                            itemReqInfo.ItemInfo!.IsRefreshRequired = false;
                        }
                    }
                }
            }

            //now that we have processed the batch request and sent responses for
            //all the items, send a response for the batch request itself
            //get a buffer for the batch status close
            ITransportBuffer? msgBuf = chnl.GetBuffer(ACK_MSG_SIZE, false, out errorInfo);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            //we close the stream the batch request was made on (and later send the
            //item responses on different streams)
            ReactorReturnCode reactorReturnCode = EncodeBatchCloseStatus(chnl, msg.DomainType, msgBuf, msg.StreamId, dataState, out errorInfo);
            if (reactorReturnCode != ReactorReturnCode.SUCCESS)
            {
                return reactorReturnCode;
            }

            //send batch status close
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

        private ReactorReturnCode EncodeBatchCloseStatus(ReactorChannel chnl, int domainType, ITransportBuffer msgBuf, int streamId, int dataState, out ReactorErrorInfo? errorInfo)
        {
            m_StatusMsg.Clear();

            // set-up message
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.StreamId = streamId;
            m_StatusMsg.DomainType = domainType;
            m_StatusMsg.ContainerType = DataTypes.NO_DATA;
            m_StatusMsg.Flags = StatusMsgFlags.HAS_STATE;
            m_StatusMsg.State.StreamState(StreamStates.CLOSED);
            m_StatusMsg.State.DataState(dataState);
            m_StatusMsg.State.Code(StateCodes.NONE);
            m_StatusMsg.State.Text().Data("Stream closed for batch");

            // clear encode iterator
            m_EncodeIter.Clear();

            // encode message
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }
            ret = m_StatusMsg.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"StatusMsg.Encode() failed with the return code {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode ProcessSingleItemRequest(ReactorChannel chnl, Msg msg, DecodeIterator dIter, bool isPrivateStream, out ReactorErrorInfo? errorInfo)
        {
            int domainType = msg.DomainType;

            //check for private stream special item name without private stream
            //flag set
            if (!isPrivateStream && m_PrivateStreamItemName.Equals(msg.MsgKey.Name))
            {
                return SendItemRequestReject(chnl, msg.StreamId, domainType, ItemRejectReason.PRIVATE_STREAM_REDIRECT, isPrivateStream, out errorInfo);
            }

            //check for invalid symbol list request
            if ((domainType == (int)DomainType.SYMBOL_LIST) && (msg.MsgKey.Name != null))
            {
                //if the consumer specified symbol list name isn't
                //"_ETA_ITEM_LIST", reject it
                if (!msg.MsgKey.Name.Equals(m_slNameBuf))
                {
                    return SendItemRequestReject(chnl, msg.StreamId, domainType, ItemRejectReason.ITEM_NOT_SUPPORTED, isPrivateStream, out errorInfo);
                }
            }

            HashSet<ItemRejectReason> rejectReasonSet = new HashSet<ItemRejectReason>();

            ItemRequestInfo? itemReqInfo = GetMatchingItemReqInfo(chnl.Channel!, msg, msg.StreamId, rejectReasonSet);
            ItemRejectReason itemReject = rejectReasonSet.Count > 0 ? rejectReasonSet.ElementAt(0) : ItemRejectReason.NONE;

            if (itemReqInfo == null && itemReject == ItemRejectReason.NONE)
            {
                //No matching items. This is a new request.
                rejectReasonSet.Clear();
                itemReqInfo = GetNewItemReqInfo(chnl, msg, msg.StreamId, rejectReasonSet, out errorInfo);
            }

            if (itemReqInfo == null)
            {
                return SendItemRequestReject(chnl, msg.StreamId, domainType, rejectReasonSet.ElementAt(0), isPrivateStream, out errorInfo);
            }

            if (msg.CheckPrivateStream())
            {
                Console.WriteLine($"Received Private Stream Item Request for {itemReqInfo.ItemName} (streamId={msg.StreamId}) on domain {DomainTypes.ToString(itemReqInfo.DomainType)}");
            }
            else
            {
                Console.WriteLine($"Received Item Request for {itemReqInfo.ItemName} (streamId={msg.StreamId}) on domain {DomainTypes.ToString(itemReqInfo.DomainType)}");
            }

            //send item refresh
            if (!msg.CheckNoRefresh())
            {
                itemReqInfo.ItemInfo!.IsRefreshRequired = true;
                ReactorReturnCode ret = SendItemResponse(chnl, itemReqInfo, out errorInfo);
                if (ret != ReactorReturnCode.SUCCESS)
                {
                    return ReactorReturnCode.FAILURE;
                }
            }

            if (!itemReqInfo.IsStreamingRequest)
            {
                //snapshot request - so we dont have to send updates
                //free item request info
                FreeItemReqInfo(itemReqInfo, out errorInfo);
            }
            else
            {
                itemReqInfo.ItemInfo!.IsRefreshRequired = false;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private void CloseStream(IChannel chnl, int streamId)
        {
            ItemRequestInfo? itemRequestInfo = m_ItemRequestWatchList.Get(chnl, streamId);
            //remove original item request information
            if (itemRequestInfo != null)
            {
                Console.WriteLine($"Closing item stream id {itemRequestInfo.StreamId} with item name: {itemRequestInfo.ItemName}");
                FreeItemReqInfo(itemRequestInfo, out ReactorErrorInfo? errorInfo);
                if (errorInfo != null)
                {
                    Console.WriteLine($"Closing item stream id {itemRequestInfo.StreamId} failed: {errorInfo.Error.Text}");
                }
            }
            else
            {
                Console.WriteLine($"No item found for StreamId: {streamId}");
            }
        }

        private void DeleteSymbolListItem(ItemRequestInfo itemReqInfo, out ReactorErrorInfo? errorInfo)
        {
            Buffer delItem = itemReqInfo.ItemName;
            errorInfo = null;

            // TRI and RES-DS are always present in our symbol list and should never be deleted
            if (m_TriItemName.Equals(delItem) || m_PrivateStreamItemName.Equals(delItem))
            {
                return;
            }

            //search the symbol list, and delete the item if the interest count is 0
            for (int i = 2; i < SymbolListItems.MAX_SYMBOL_LIST_SIZE; i++)
            {
                if (delItem.Equals(m_SymbolListItemWatchList.SymbolListItemName(i)))
                {
                    m_SymbolListItemWatchList.DecrementInterestCount(i);

                    //no more interest in the item, so remove it from the symbol list
                    if (m_SymbolListItemWatchList.InterestCount(i) == 0)
                    {
                        m_SymbolListItemWatchList.Clear(i);
                        m_SymbolListItemWatchList.DecrementItemCount();

                        //find all consumers using the symbol list domain and send them updates
                        foreach (ItemRequestInfo itemReqInfoL in m_ItemRequestWatchList)
                        {
                            //Only send Symbol List updates to active channels that have made requests
                            if (itemReqInfoL.DomainType == (int)DomainType.SYMBOL_LIST && itemReqInfoL.Channel!.State == ChannelState.ACTIVE)
                            {
                                SendSLItemUpdates(itemReqInfoL.ReactorChannel!, itemReqInfo, SymbolListItems.SYMBOL_LIST_UPDATE_DELETE, itemReqInfoL.StreamId, out errorInfo);
                            }
                        }
                    }
                    break;
                }
            }
        }

        private void FreeItemReqInfo(ItemRequestInfo itemReqInfo, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            if (itemReqInfo != null)
            {
                //decrement item interest count
                if (itemReqInfo.ItemInfo != null && itemReqInfo.ItemInfo.InterestCount > 0)
                {
                    itemReqInfo.ItemInfo.InterestCount = itemReqInfo.ItemInfo.InterestCount - 1;
                }

                if (itemReqInfo.DomainType != (int)DomainType.SYMBOL_LIST && itemReqInfo.ItemInfo!.InterestCount == 0)
                {
                    DeleteSymbolListItem(itemReqInfo, out errorInfo);
                    if (errorInfo != null)
                    {
                        Console.WriteLine($"Error deleting symbol list: {errorInfo.Error.Text}");
                    }
                }

                //free item information if no more interest
                if (itemReqInfo.ItemInfo!.InterestCount == 0)
                {
                    itemReqInfo.ItemInfo.Clear();
                }

                //free item request information
                itemReqInfo.Clear();
            }
        }

        private ReactorReturnCode SendCloseStatus(ReactorChannel channel, ItemRequestInfo itemReqInfo, out ReactorErrorInfo? errorInfo)
        {
            //get a buffer for the close status
            ITransportBuffer? msgBuf = channel.GetBuffer(1024, false, out errorInfo);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            //encode close status
            m_MarketPriceStatus.Clear();
            m_MarketPriceStatus.StreamId = itemReqInfo.StreamId;
            m_MarketPriceStatus.DomainType = itemReqInfo.DomainType;
            if (itemReqInfo.IsPrivateStreamRequest)
                m_MarketPriceStatus.PrivateStream = true;
            m_MarketPriceStatus.HasState = true;
            m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
            m_MarketPriceStatus.State.DataState(DataStates.SUSPECT);
            m_MarketPriceStatus.State.Text().Data($"Stream closed for item: {itemReqInfo.ItemName}");

            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            ret = m_MarketPriceStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"MarketPriceStatus.Encode() failed: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            return channel.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

        private ReactorReturnCode EncodeAck(ITransportBuffer msgBuf, ReactorChannel chnl, IPostMsg postMsg, int nakCode, string text, out ReactorErrorInfo? errorInfo)
        {
            //set-up message 
            m_AckMsg.MsgClass = MsgClasses.ACK;
            m_AckMsg.StreamId = postMsg.StreamId;
            m_AckMsg.DomainType = postMsg.DomainType;
            m_AckMsg.ContainerType = DataTypes.NO_DATA;
            m_AckMsg.Flags = AckMsgFlags.NONE;
            m_AckMsg.NakCode = nakCode;
            m_AckMsg.AckId = postMsg.PostId;
            m_AckMsg.SeqNum = postMsg.SeqNum;

            if (nakCode != NakCodes.NONE)
                m_AckMsg.ApplyHasNakCode();

            if (postMsg.CheckHasSeqNum())
                m_AckMsg.ApplyHasSeqNum();

            if (text != null)
            {
                m_AckMsg.ApplyHasText();
                m_AckMsg.Text.Data(text);
            }

            //encode message
            m_EncodeIter.Clear();

            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            ret = m_AckMsg.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "AckMsg.Encode() failed with return code {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private ItemRequestInfo? GetMatchingItemReqInfo(IChannel channel, Msg msg, int streamId, HashSet<ItemRejectReason> rejectReason)
        {
            foreach (ItemRequestInfo itemRequestInfo in m_ItemRequestWatchList)
            {
                if (itemRequestInfo.IsInUse && itemRequestInfo.Channel == channel)
                {
                    if (itemRequestInfo.DomainType == msg.DomainType
                            && (itemRequestInfo.MsgKey.Equals(msg.MsgKey)))
                    {
                        // The request has the same domain and key as one currently
                        // open for this channel.
                        if (itemRequestInfo.StreamId != streamId)
                        {
                            // The request has a different stream ID, meaning it
                            // would open the same item on another stream.
                            // This is not allowed (except for private streams).
                            if (!msg.CheckPrivateStream())
                            {
                                rejectReason.Add(ItemRejectReason.ITEM_ALREADY_OPENED);
                                return null;
                            }
                            // Otherwise continue checking the list
                        }
                        else
                        {
                            // Check that the private stream flag matches correctly.
                            if (msg.CheckPrivateStream() && !itemRequestInfo.IsPrivateStreamRequest || !msg.CheckPrivateStream() && itemRequestInfo.IsPrivateStreamRequest)
                            {
                                // This item would be a match except that the
                                // private stream flag does not match.
                                rejectReason.Add(ItemRejectReason.PRIVATE_STREAM_MISMATCH);
                                return null;
                            }

                            // The domain, key, stream ID, and private stream flag
                            // all match, so this item is a match, and the request
                            // is a reissue.
                            return itemRequestInfo;
                        }
                    }
                    else if (itemRequestInfo.StreamId == streamId)
                    {
                        // This stream ID is already in use for a different item.
                        rejectReason.Add(ItemRejectReason.STREAM_ALREADY_IN_USE);
                        return null;
                    }
                }
            }

            rejectReason.Add(ItemRejectReason.NONE);
            return null;
        }

        private ReactorReturnCode SendAck(ReactorChannel chnl, IPostMsg postMsg, int nakCode, string errText, out ReactorErrorInfo? errorInfo)
        {
            //send an ack if it was requested
            if (postMsg.CheckAck())
            {
                ITransportBuffer? msgBuf = chnl.GetBuffer(ACK_MSG_SIZE, false, out errorInfo);
                if (msgBuf == null)
                {
                    return ReactorReturnCode.FAILURE;
                }

                ReactorReturnCode ret = EncodeAck(msgBuf, chnl, postMsg, nakCode, errText, out errorInfo);
                if (ret != ReactorReturnCode.SUCCESS)
                    return ret;

                return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private ItemRequestInfo? GetNewItemReqInfo(ReactorChannel channel, Msg msg, int stream, HashSet<ItemRejectReason> rejectReasons, out ReactorErrorInfo? errorInfo)
        {
            ItemRequestInfo? itemRequestInfo = null;
            int count = 0;
            errorInfo = null;

            //Find an available item request info structure to use, and check that
            //the channel has not reached its allowed limit of open items.
            foreach (ItemRequestInfo itemReqInfo in m_ItemRequestWatchList)
            {
                if (itemReqInfo.IsInUse)
                {
                    if (itemReqInfo.Channel == channel.Channel)
                    {
                        ++count;
                        if (count >= DirectoryHandler.OPEN_LIMIT)
                        {
                            //Consumer has requested too many items.
                            rejectReasons.Add(ItemRejectReason.ITEM_COUNT_REACHED);
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
                rejectReasons.Add(ItemRejectReason.ITEM_COUNT_REACHED);
                return null;
            }

            itemRequestInfo.ReactorChannel = channel;
            itemRequestInfo.Channel = channel.Channel;
            itemRequestInfo.IsInUse = true;
            if (CopyMsgKey(itemRequestInfo.MsgKey, msg.MsgKey) != CodecReturnCode.SUCCESS)
            {
                rejectReasons.Add(ItemRejectReason.ITEM_NOT_SUPPORTED);
                return null;
            }

            itemRequestInfo.DomainType = msg.DomainType;
            // copy item name buffer
            BufferHelper.CopyBuffer(itemRequestInfo.MsgKey.Name, itemRequestInfo.ItemName);
            int msgFlags = msg.Flags;
            if ((msgFlags & (int)RequestMsgFlags.PRIVATE_STREAM) != 0)
            {
                itemRequestInfo.IsPrivateStreamRequest = true;
            }

            // get IsStreamingRequest
            if ((msgFlags & (int)RequestMsgFlags.STREAMING) != 0)
            {
                itemRequestInfo.IsStreamingRequest = true;
            }

            // get IncludeKeyInUpdates
            if ((msgFlags & (int)RequestMsgFlags.MSG_KEY_IN_UPDATES) != 0)
            {
                itemRequestInfo.IncludeKeyInUpdates = true;
            }

            //get item information
            itemRequestInfo.ItemInfo = m_ItemInfoWatchList.Get(channel.Channel!, itemRequestInfo.ItemName, itemRequestInfo.DomainType, itemRequestInfo.IsPrivateStreamRequest);
            if (itemRequestInfo.ItemInfo == null)
            {
                rejectReasons.Add(ItemRejectReason.ITEM_NOT_SUPPORTED);
            }
            else
            {
                switch (itemRequestInfo.DomainType)
                {
                    case (int)DomainType.MARKET_PRICE:
                        itemRequestInfo.ItemInfo.ItemData = m_MarketPriceItemWatchList.Get(itemRequestInfo.ItemName.ToString());
                        break;
                    case (int)DomainType.MARKET_BY_ORDER:
                        itemRequestInfo.ItemInfo.ItemData = m_MarketByOrderItemWatchList.Get(itemRequestInfo.ItemName.ToString());
                        break;
                    case (int)DomainType.MARKET_BY_PRICE:
                        itemRequestInfo.ItemInfo.ItemData = m_MarketByPriceItemWatchList.Get(itemRequestInfo.ItemName.ToString());
                        break;
                    case (int)DomainType.SYMBOL_LIST:
                        break;
                    default:
                        break;
                }

                if ((itemRequestInfo.ItemInfo.ItemData == null) && (itemRequestInfo.DomainType != (int)DomainType.SYMBOL_LIST))
                {
                    rejectReasons.Add(ItemRejectReason.ITEM_COUNT_REACHED);
                    return null;
                }

                if (itemRequestInfo.ItemInfo.DomainType != (int)DomainType.SYMBOL_LIST)
                {
                    AddSymbolListItem(channel, itemRequestInfo);
                }
            }
            //get IsStreamingRequest
            if (msg.CheckStreaming())
            {
                itemRequestInfo.IsStreamingRequest = true;
            }

            //IsPrivateStreamRequest
            if (msg.CheckPrivateStream())
            {
                itemRequestInfo.IsPrivateStreamRequest = true;
            }

            //IncludeKeyInUpdates
            if (msg.CheckMsgKeyInUpdates())
            {
                itemRequestInfo.IncludeKeyInUpdates = true;
            }

            //increment item interest count if new request
            itemRequestInfo.ItemInfo!.InterestCount = itemRequestInfo.ItemInfo.InterestCount + 1;
            itemRequestInfo.StreamId = stream;

            //provide a refresh if one was requested.
            itemRequestInfo.ItemInfo.IsRefreshRequired = msg.CheckNoRefresh() ? false : true;

            return itemRequestInfo;
        }

        private void AddSymbolListItem(ReactorChannel channel, ItemRequestInfo itemReqInfo)
        {
            int itemVacancy = 0;
            Buffer newItem = itemReqInfo.ItemName;
            bool foundVacancy = false;

            //TRI and RES-DS are added to our symbol list at initialization,
            //and are always present so they never need to be added again
            if (m_TriItemName.Equals(newItem) || m_PrivateStreamItemName.Equals(newItem) || m_ItemCount >= SymbolListItems.MAX_SYMBOL_LIST_SIZE)
            {
                return;
            }

            //check to see if this item is already in the item list
            for (int i = 2; i < SymbolListItems.MAX_SYMBOL_LIST_SIZE; i++)
            {
                //if the item is already present, increment the interest count
                if (newItem.Equals(m_SymbolListItemWatchList.SymbolListItemName(i)))
                {
                    m_SymbolListItemWatchList.IncrementInterestCount(i);
                    return;
                }
                if ((m_SymbolListItemWatchList.GetStatus(i) == false) && foundVacancy == false)
                {
                    //store the index of the first vacancy in the symbol list
                    foundVacancy = true;
                    itemVacancy = i;
                }
            }

            //add the new item name to the symbol list 
            m_SymbolListItemWatchList.SymbolListItemName(newItem, itemVacancy);
            m_SymbolListItemWatchList.IncrementInterestCount(itemVacancy);

            // find all consumers currently using the symbol list domain, and send them updates
            foreach (ItemRequestInfo itemReqInfoL in m_ItemRequestWatchList)
            {
                if (itemReqInfoL.DomainType == (int)DomainType.SYMBOL_LIST)
                {
                    SendSLItemUpdates(itemReqInfoL.ReactorChannel!, itemReqInfo, SymbolListItems.SYMBOL_LIST_UPDATE_ADD, itemReqInfoL.StreamId, out ReactorErrorInfo? errorInfo);
                    if (errorInfo != null)
                    {
                        Console.WriteLine($"Failed to send updates for stream id {itemReqInfoL.StreamId}: {(errorInfo != null ? errorInfo.Error.Text : "")}");
                    }
                }
            }
        }

        private void SendSLItemUpdates(ReactorChannel reactorChannel, ItemRequestInfo itemReqInfo, int responseType, int streamId, out ReactorErrorInfo? errorInfo)
        {
            //get a buffer for the response
            ITransportBuffer msgBuf = reactorChannel.Channel!.GetBuffer(SymbolListItems.MAX_SYMBOL_LIST_SIZE, false, out m_Error);
            if (msgBuf == null)
            {
                Console.WriteLine($"chnl.GetBuffer() failed, error: {m_Error?.Text}");
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"chnl.GetBuffer() failed, error: {m_Error?.Text}";
                return;
            }

            CodecReturnCode ret = m_SymbolListItemWatchList.EncodeResponse(reactorChannel.Channel, itemReqInfo.ItemInfo!,
                msgBuf, streamId, true, ServiceId, itemReqInfo.IsStreamingRequest, m_DictionaryHandler.Dictionary, responseType, out Error? error);

            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeSymbolListResponse() failed, error: {error?.Text}");
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"EncodeSymbolListResponse() failed, error: {error?.Text}";
                return;
            }

            if (reactorChannel.Submit(msgBuf, m_SubmitOptions, out errorInfo) != ReactorReturnCode.SUCCESS)
            {
                Console.WriteLine($"ReactorChannel.Sumit() failed, error: {errorInfo?.Error?.Text}");
                return;
            }

            errorInfo = null;
        }

    }
}
