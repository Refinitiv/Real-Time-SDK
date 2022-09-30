/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using Refinitiv.Eta.Codec;
using Buffer = Refinitiv.Eta.Codec.Buffer;
using Refinitiv.Eta.Example.Common;
using Refinitiv.Eta.Transports;
using Array = Refinitiv.Eta.Codec.Array;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.Rdm;
using System.Collections.Generic;
using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Common;

namespace Refinitiv.Eta.Example.Provider
{
    /// <summary>
    /// This is the implementation of handling item requests for the interactive provider application.
    /// <remarks>
    /// It provides methods for processing item requests from consumers and sending
    /// back the refresh/update messages. Methods for sending request reject/close
    /// status messages, initializing the item handler, checking if the item count
    /// per channel has been reached, checking if an item is already opened on a
    /// channel, checking if a stream is already in use, and closing item streams are
    /// also provided.
    /// This handler provides data for MarketPrice, MarketByPrice and MarketByOrder
    /// item requests.
    /// </remarks>
    /// </summary>
    public class ItemHandler
    {
        private const int REJECT_MSG_SIZE = 1024;
        private const int ACK_MSG_SIZE = 1024;
        private const int ITEM_MSG_SIZE = 1024;
        private const int MAX_REFRESH_PARTS = 3;
        private const int POST_MSG_SIZE = 1024;
        private static Buffer m_TriItemName = new Buffer();
        private static Buffer m_PrivateStreamItemName = new Buffer();
        private static Buffer m_SlNameBuf = new Buffer();
        private static Buffer m_BatchReqName = new Buffer();

        private MarketPriceStatus m_MarketPriceStatus;
        private ProviderSession m_ProviderSession;
        private SymbolListItems m_SymbolListItemWatchList;
        private MarketByOrderItems m_MarketByOrderItemWatchList;
        private MarketByPriceItems m_MarketByPriceItemWatchList;
        private MarketPriceItems m_MarketPriceItemWatchList;
        private ItemInfoList m_ItemInfoWatchList;
        private ItemRequestInfoList m_ItemRequestWatchList;

        private ProviderDictionaryHandler m_DictionaryHandler;
        private ProviderLoginHandler m_LoginHandler;

        public int ServiceId { get; set; }
        private int m_ItemCount = 0;

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

        /// <summary>
        /// Instantiates a new item handler.
        /// </summary>
        /// <param name="providerSession">The provider session</param>
        /// <param name="dictionaryHandler">The dictionary handler</param>
        /// <param name="loginHandler">The login handler</param>
        public ItemHandler(ProviderSession providerSession, ProviderDictionaryHandler dictionaryHandler, ProviderLoginHandler loginHandler)
        {
            m_ProviderSession = providerSession;
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
            m_SlNameBuf.Data("_ETA_ITEM_LIST");
            m_BatchReqName.Data(":ItemList");

            //set Qos for provider
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
        /// Processes an item request. This consists of storing the request
        /// information, then calling sendItemResponse() to send the response.
        /// </summary>
        /// <param name="chnl">The channel of the response</param>
        /// <param name="msg">The partially decoded message</param>
        /// <param name="dIter">The decode iterator</param>
        /// <param name="error">The error</param>
        /// <returns>returns success if decoding of request message and sending of
        /// response message succeeds or failure if it fails
        /// </returns>
        public CodecReturnCode ProcessRequest(IChannel chnl, Msg msg, DecodeIterator dIter, out Error? error)
        {
            error = null;
            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    if (!msg.MsgKey.CheckHasServiceId() || msg.MsgKey.ServiceId != ServiceId)
                    {
                        return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.INVALID_SERVICE_ID, false, out error);
                    }

                    //check if QoS supported 
                    if ((msg.Flags & RequestMsgFlags.HAS_WORST_QOS) != 0 &&
                            (msg.Flags & RequestMsgFlags.HAS_QOS) != 0)
                    {
                        if (!m_ProviderQos.IsInRange(msg.Qos, msg.WorstQos))
                        {
                            return SendItemRequestReject(chnl, msg.StreamId,
                                                    msg.DomainType, ItemRejectReason.QOS_NOT_SUPPORTED, false, out error);
                        }
                    }
                    else if ((msg.Flags & RequestMsgFlags.HAS_QOS) != 0)
                    {
                        if (!msg.Qos.Equals(m_ProviderQos))
                        {
                            return SendItemRequestReject(chnl, msg.StreamId,
                                                    msg.DomainType, ItemRejectReason.QOS_NOT_SUPPORTED, false, out error);
                        }
                    }

                    //check for unsupported key attribute information
                    if (msg.MsgKey.CheckHasAttrib())
                    {
                        return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.KEY_ENC_ATTRIB_NOT_SUPPORTED, false, out error);
                    }

                    if (msg.CheckHasBatch())
                    {
                        return ProcessBatchRequest(chnl, msg, dIter, msg.CheckPrivateStream(), out error);
                    }
                    else
                    {
                        return ProcessSingleItemRequest(chnl, msg, dIter, msg.CheckPrivateStream(), out error);
                    }
                case MsgClasses.CLOSE:
                    Console.WriteLine($"Received Item close for streamId {msg.StreamId}");
                    CloseStream(chnl, msg.StreamId);
                    break;
                case MsgClasses.POST:
                    return ProcessPost(chnl, msg, dIter, out error);
                default:
                    {
                        error = new Error
                        {
                            ErrorId = TransportReturnCode.FAILURE,
                            Text = $"Received Unhandled Item Msg Class: {MsgClasses.ToString(msg.MsgClass)}"
                        };

                        return CodecReturnCode.FAILURE;
                    }
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode ProcessBatchRequest(IChannel chnl, Msg msg, DecodeIterator dIter, bool isPrivateStream, out Error? error)
        {
            Console.WriteLine($"Received batch item request (streamId={msg.StreamId}) on domain {DomainTypes.ToString(msg.DomainType)}");

            /* check if batch stream already in use with a different key */
            if (IsStreamInUse(chnl, msg.StreamId, msg.MsgKey))
            {
                return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.STREAM_ALREADY_IN_USE, isPrivateStream, out error);
            }

            // The payload of a batch request contains an elementList
            CodecReturnCode ret = m_ElementList.Decode(dIter, null);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.REQUEST_DECODE_ERROR, isPrivateStream, out error);
            }

            HashSet<ItemRejectReason> rejectReasonSet = new HashSet<ItemRejectReason>();
            int dataState = DataStates.NO_CHANGE;
            // The list of items being requested is in an elementList entry with the
            // element name of ":ItemList"
            while ((ret = m_ElementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (m_ElementEntry.Name.Equals(m_BatchReqName))
                {
                    // The list of items names is in an array
                    ret = m_Array.Decode(dIter);
                    if (ret<CodecReturnCode.SUCCESS)
                    {
                        error = new Error
                        {
                            ErrorId = TransportReturnCode.FAILURE,
                            Text = $"Array.Decode() for batch request failed with return code: {ret.GetAsString()}"
                        };

                        break;
                    }

                    // Get each requested item name
                    // We will assign consecutive stream IDs for each item
                    // starting with the stream following the one the batch request
                    // was made on
                    int itemStream = msg.StreamId;

                    while ((ret = m_ArrayEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
                    {
                        if (ret<CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"ArrayEntry.Decode() failed with return code: {ret.GetAsString()}"
                            };

                            dataState = DataStates.SUSPECT;
                            continue;
                        }

                        //check if stream already in use with a different key
                        itemStream++; // increment stream ID with each item we find
                        // in the batch request

                        //check for private stream special item name without
                        //private stream flag set
                        if (!isPrivateStream && m_ArrayEntry.EncodedData.Equals(m_PrivateStreamItemName))
                        {
                            SendItemRequestReject(chnl, itemStream, msg.DomainType, ItemRejectReason.PRIVATE_STREAM_REDIRECT, isPrivateStream, out error);
                            dataState = DataStates.SUSPECT;
                            continue;
                        }

                        //all of the items requested have the same key. They use
                        //the key of the batch request.
                        //The only difference is the name
                        msg.MsgKey.Flags = msg.MsgKey.Flags | MsgKeyFlags.HAS_NAME;
                        msg.MsgKey.Name = m_ArrayEntry.EncodedData;

                        ItemRequestInfo? itemReqInfo = GetMatchingItemReqInfo(chnl, msg, itemStream, rejectReasonSet);
                        ItemRejectReason rejectReason = rejectReasonSet.GetEnumerator().MoveNext() ?
                            rejectReasonSet.GetEnumerator().Current : ItemRejectReason.NONE;
                        if (itemReqInfo == null && rejectReason != ItemRejectReason.NONE)
                        {
                            SendItemRequestReject(chnl, itemStream, msg.DomainType, rejectReason, isPrivateStream, out error);
                            dataState = DataStates.SUSPECT;
                            continue;
                        }
                        else if (itemReqInfo != null)
                        {
                            // Batch requests should not be used to reissue item
                            // requests.
                            SendItemRequestReject(chnl, itemStream, msg.DomainType, ItemRejectReason.BATCH_ITEM_REISSUE, isPrivateStream, out error);
                            dataState = DataStates.SUSPECT;
                            continue;
                        }

                        rejectReasonSet.Clear();
                        itemReqInfo = GetNewItemReqInfo(chnl, msg, itemStream, rejectReasonSet);
                        if (itemReqInfo == null)
                        {
                            // Batch requests should not be used to reissue item
                            // requests.
                            SendItemRequestReject(chnl, itemStream, msg.DomainType, rejectReasonSet.GetEnumerator().Current, isPrivateStream, out error);
                            dataState = DataStates.SUSPECT;
                            continue;
                        }

                        if (msg.CheckPrivateStream())
                        {
                            Console.WriteLine($"Received Private Stream Item Request for {itemReqInfo.ItemName} (streamId={itemStream}) on domain {DomainTypes.ToString(itemReqInfo.DomainType)}");
                        }
                        else
                        {
                            Console.WriteLine($"Received Item Request for {itemReqInfo.ItemName} (streamId={itemStream}) on domain {DomainTypes.ToString(itemReqInfo.DomainType)}");
                        }

                        //send item response/refresh if required
                        if (!msg.CheckNoRefresh())
                            SendItemResponse(itemReqInfo, out error);

                        if (!itemReqInfo.IsStreamingRequest)
                        {
                            //snapshot request - so we dont have to send updates
                            //free item request info
                            FreeItemReqInfo(itemReqInfo);
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
            ITransportBuffer msgBuf = chnl.GetBuffer(ACK_MSG_SIZE, false, out error);
            if (msgBuf is null)
            {
                return CodecReturnCode.FAILURE;
            }

            //we close the stream the batch request was made on (and later send the
            //item responses on different streams)
            ret = EncodeBatchCloseStatus(chnl, msg.DomainType, msgBuf, msg.StreamId, dataState, out error);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            //send batch status close
            return (CodecReturnCode)m_ProviderSession.Write(chnl, msgBuf, out error);
        }

        private CodecReturnCode EncodeBatchCloseStatus(IChannel chnl, int domainType, ITransportBuffer msgBuf, int streamId, int dataState, out Error? error)
        {
            error = null;
            m_StatusMsg.Clear();

            /* set-up message */
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.StreamId = streamId;
            m_StatusMsg.DomainType = domainType;
            m_StatusMsg.ContainerType = DataTypes.NO_DATA;
            m_StatusMsg.Flags = StatusMsgFlags.HAS_STATE;
            m_StatusMsg.State.StreamState(StreamStates.CLOSED);
            m_StatusMsg.State.DataState(dataState);
            m_StatusMsg.State.Code(StateCodes.NONE);
            m_StatusMsg.State.Text().Data("Stream closed for batch");

            /* clear encode iterator */
            m_EncodeIter.Clear();

            /* encode message */
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"EncodeIterator.setBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }
            ret = m_StatusMsg.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"StatusMsg.Encode() failed"
                };

                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode ProcessSingleItemRequest(IChannel chnl, Msg msg, DecodeIterator dIter, bool isPrivateStream, out Error? error)
        {
            error = null;
            int domainType = msg.DomainType;

            //check for private stream special item name without private stream
            //flag set
            if (!isPrivateStream && m_PrivateStreamItemName.Equals(msg.MsgKey.Name))
            {
                return SendItemRequestReject(chnl, msg.StreamId, domainType, ItemRejectReason.PRIVATE_STREAM_REDIRECT, isPrivateStream, out error);
            }

            //check for invalid symbol list request
            if ((domainType == (int)DomainType.SYMBOL_LIST) && (msg.MsgKey.Name != null))
            {
                //if the consumer specified symbol list name isn't
                //"_ETA_ITEM_LIST", reject it
                if (!msg.MsgKey.Name.Equals(m_SlNameBuf))
                {
                    return SendItemRequestReject(chnl, msg.StreamId, domainType, ItemRejectReason.ITEM_NOT_SUPPORTED, isPrivateStream, out error);
                }
            }

            //get request info structure
            //check if item already opened with exact same key on another stream 
            HashSet<ItemRejectReason> rejectReasonSet = new HashSet<ItemRejectReason>();
        
            //Check for reissue request
            ItemRequestInfo? itemReqInfo = GetMatchingItemReqInfo(chnl, msg, msg.StreamId, rejectReasonSet);
            ItemRejectReason itemReject = (rejectReasonSet.GetEnumerator().MoveNext() ? rejectReasonSet.GetEnumerator().Current : ItemRejectReason.NONE);
            if (itemReqInfo == null && itemReject == ItemRejectReason.NONE)
            {
                //No matching items. This is a new request.
                rejectReasonSet.Clear();
                itemReqInfo = GetNewItemReqInfo(chnl, msg, msg.StreamId, rejectReasonSet);
            }

            if (itemReqInfo == null)
            {
                return SendItemRequestReject(chnl, msg.StreamId, domainType, rejectReasonSet.GetEnumerator().Current, isPrivateStream, out error);
            }

            if (msg.CheckPrivateStream())
            {
                Console.WriteLine($"Received Private Stream Item Request for {itemReqInfo.ItemName}(streamId={ msg.StreamId}) on domain {DomainTypes.ToString(itemReqInfo.DomainType)}");
            }
            else
            {
                Console.WriteLine($"Received Item Request for {itemReqInfo.ItemName}(streamId={msg.StreamId}) on domain {DomainTypes.ToString(itemReqInfo.DomainType)}");
            }

            //send item refresh
            if (!msg.CheckNoRefresh())
            {
                itemReqInfo.ItemInfo!.IsRefreshRequired = true;
                CodecReturnCode ret = SendItemResponse(itemReqInfo, out error);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (!itemReqInfo.IsStreamingRequest) 
            {
                //snapshot request - so we dont have to send updates
                //free item request info
                FreeItemReqInfo(itemReqInfo);
            }
            else
            {
                itemReqInfo.ItemInfo!.IsRefreshRequired = false;
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode SendItemRequestReject(IChannel channel, int streamId, int domainType, ItemRejectReason reason, bool isPrivateStream, out Error? error)
        {
            error = null;
            //get a buffer for the item request reject status
            ITransportBuffer msgBuf = channel.GetBuffer(REJECT_MSG_SIZE, false, out error);
            if (msgBuf == null)
                return CodecReturnCode.FAILURE;

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
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId}- item count reached for this channel");
                    break;
                case ItemRejectReason.INVALID_SERVICE_ID:
                    m_MarketPriceStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId}- service id invalid");
                    break;
                case ItemRejectReason.QOS_NOT_SUPPORTED:
                    m_MarketPriceStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId}- QoS not supported");
                    break;
                case ItemRejectReason.ITEM_ALREADY_OPENED:
                    m_MarketPriceStatus.State.Code(StateCodes.ALREADY_OPEN);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId}- item already open with exact same key on another stream");
                    break;
                case ItemRejectReason.STREAM_ALREADY_IN_USE:
                    m_MarketPriceStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId}- stream already in use with a different key");
                    break;
                case ItemRejectReason.KEY_ENC_ATTRIB_NOT_SUPPORTED:
                    m_MarketPriceStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId}- this provider does not support key attribute information");
                    break;
                case ItemRejectReason.ITEM_NOT_SUPPORTED:
                    m_MarketPriceStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
                    m_MarketPriceStatus.State.Text().Data($"Item request rejected for stream id {streamId}- item not supported");
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
                default:
                    break;
            }

            m_EncodeIter.Clear();
            m_MarketPriceStatus.StreamId = streamId;
            m_MarketPriceStatus.DomainType = domainType;
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            ret = m_MarketPriceStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"MarketPriceStatus.Encode() failed"
                };

                return ret;
            }

            Console.WriteLine($"Rejecting Item Request with streamId={streamId} and domain {DomainTypes.ToString(domainType)}.  Reason: {reason.GetAsString()}");

            return (CodecReturnCode)m_ProviderSession.Write(channel, msgBuf, out error);
        }


        private void CloseStream(IChannel chnl, int streamId)
        {
            ItemRequestInfo? itemRequestInfo = m_ItemRequestWatchList.Get(chnl, streamId);
            //remove original item request information
            if (itemRequestInfo != null)
            {
                Console.WriteLine($"Closing item stream id {itemRequestInfo.StreamId} with item name: {itemRequestInfo.ItemName}");
                FreeItemReqInfo(itemRequestInfo);
            }
            else
            {
                Console.WriteLine($"No item found for StreamId: {streamId}");
            }
        }

        private void DeleteSymbolListItem(ItemRequestInfo itemReqInfo)
        {
            Buffer delItem = itemReqInfo.ItemName;

            // TRI and RES-DS are always present in our symbol list and should never
            // be deleted
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
                                SendSLItemUpdates(itemReqInfoL.Channel, itemReqInfo, SymbolListItems.SYMBOL_LIST_UPDATE_DELETE, itemReqInfoL.StreamId);
                            }
                        }
                    }
                    break;
                }
            }
        }

        private void FreeItemReqInfo(ItemRequestInfo itemReqInfo)
        {
            if (itemReqInfo != null)
            {
                //decrement item interest count
                if (itemReqInfo.ItemInfo != null && itemReqInfo.ItemInfo.InterestCount > 0)
                {
                    itemReqInfo.ItemInfo.InterestCount--;
                }

                if (itemReqInfo.DomainType != (int)DomainType.SYMBOL_LIST && itemReqInfo.ItemInfo!.InterestCount == 0)
                {
                    DeleteSymbolListItem(itemReqInfo);
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

        /// <summary>
        /// Sends the item close status message(s) for a channel. This consists of
        /// finding all request information for this channel and sending the close
        /// status messages to the channel.
        /// </summary>
        /// <param name="channel">The channel to send close status message(s) to</param>
        /// <param name="error">The error is set in an event of failure</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode SendCloseStatusMsgs(IChannel channel, out Error? error)
        {
            error = null;
            CodecReturnCode ret = 0;
            foreach (ItemRequestInfo itemRequestInfo in m_ItemRequestWatchList)
            {
                if (itemRequestInfo.IsInUse && itemRequestInfo.Channel == channel)
                {
                    ret = SendCloseStatus(channel, itemRequestInfo, out error);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode SendCloseStatus(IChannel channel, ItemRequestInfo itemReqInfo, out Error? error)
        {
            //get a buffer for the close status
            ITransportBuffer msgBuf = channel.GetBuffer(1024, false, out error);
            if (msgBuf == null)
                return CodecReturnCode.FAILURE;

            //encode close status
            m_MarketPriceStatus.Clear();
            m_MarketPriceStatus.StreamId = itemReqInfo.StreamId;
            m_MarketPriceStatus.DomainType = itemReqInfo.DomainType;
            if (itemReqInfo.IsPrivateStreamRequest)
                m_MarketPriceStatus.PrivateStream = true; ;
            m_MarketPriceStatus.HasState = true;
            m_MarketPriceStatus.State.StreamState(StreamStates.CLOSED);
            m_MarketPriceStatus.State.DataState(DataStates.SUSPECT);
            m_MarketPriceStatus.State.Text().Data($"Stream closed for item: {itemReqInfo.ItemName}");

            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            ret = m_MarketPriceStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"MarketPriceStatus.Encode() failed"
                };

                return ret;
            }

            return (CodecReturnCode)m_ProviderSession.Write(channel, msgBuf, out error);
        }

        private CodecReturnCode EncodeAck(ITransportBuffer msgBuf, IChannel chnl, IPostMsg postMsg, int nakCode, String text, out Error? error)
        {
            error = null;
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
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            ret = m_AckMsg.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"AckMsg.Encode() failed"
                };

                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        private ItemRequestInfo? GetMatchingItemReqInfo(IChannel channel, Msg msg, int streamId, HashSet<ItemRejectReason> rejectReason)
        {
            foreach (ItemRequestInfo itemRequestInfo in m_ItemRequestWatchList)
            {
                if (itemRequestInfo.IsInUse && object.ReferenceEquals(itemRequestInfo.Channel,channel))
                {
                    if (itemRequestInfo.DomainType == msg.DomainType
                            && (itemRequestInfo.MsgKey.Equals(msg.MsgKey)))
                    {
                        //The request has the same domain and key as one currently
                        //open for this channel.
                        if (itemRequestInfo.StreamId != streamId)
                        {
                            //The request has a different stream ID, meaning it
                            //would open the same item on another stream. This is
                            //not allowed(except for private streams).
                            if (!msg.CheckPrivateStream())
                            {
                                rejectReason.Add(ItemRejectReason.ITEM_ALREADY_OPENED);
                                return null;
                            }
                            //Otherwise continue checking the list
                        }
                        else
                        {
                            //Check that the private stream flag matches correctly.
                            if (msg.CheckPrivateStream() && !itemRequestInfo.IsPrivateStreamRequest
                                    || !(msg.CheckPrivateStream()) && itemRequestInfo.IsPrivateStreamRequest)
                            {
                                //This item would be a match except that the
                                //private stream flag does not match.
                                rejectReason.Add(ItemRejectReason.PRIVATE_STREAM_MISMATCH);
                                return null;
                            }

                            //The domain, key, stream ID, and private stream flag
                            //all match, so this item is a match, and the request
                            //is a reissue.
                            return itemRequestInfo;
                        }
                    }
                    else if (itemRequestInfo.StreamId == streamId)
                    {
                        //This stream ID is already in use for a different item.
                        rejectReason.Add(ItemRejectReason.STREAM_ALREADY_IN_USE);
                        return null;
                    }
                }
            }

            rejectReason.Add(ItemRejectReason.NONE);
            return null;
        }

        private CodecReturnCode SendAck(IChannel chnl, IPostMsg postMsg, int nakCode, string? errText, out Error? error)
        {
            error = null;
            //send an ack if it was requested
            if (postMsg.CheckAck())
            {
                ITransportBuffer msgBuf = chnl.GetBuffer(ACK_MSG_SIZE, false, out error);
                if (msgBuf == null)
                {
                    return CodecReturnCode.FAILURE;
                }

                CodecReturnCode ret = EncodeAck(msgBuf, chnl, postMsg, nakCode, errText!, out error);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                return (CodecReturnCode)m_ProviderSession.Write(chnl, msgBuf, out error);
            }

            return CodecReturnCode.SUCCESS;
        }

        private ItemRequestInfo? GetNewItemReqInfo(IChannel channel, Msg msg, int stream, HashSet<ItemRejectReason> rejectReasons)
        {
            ItemRequestInfo? itemRequestInfo = null;
            int count = 0;


            //Find an available item request info structure to use, and check that
            //the channel has not reached its allowed limit of open items.
            foreach (ItemRequestInfo itemReqInfo in m_ItemRequestWatchList)
            {
                if (itemReqInfo.IsInUse)
                {
                    if (itemReqInfo.Channel == channel)
                    {
                        ++count;
                        if (count >= ProviderDirectoryHandler.OPEN_LIMIT)
                        {
                            //Consumer has requested too many items.
                            rejectReasons.Add(ItemRejectReason.ITEM_COUNT_REACHED);
                            return null;
                        }
                    }
                }
                else if (itemRequestInfo is null)
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

            itemRequestInfo.Channel = channel;
            itemRequestInfo.IsInUse = true;
            if (CopyMsgKey(itemRequestInfo.MsgKey, msg.MsgKey) != CodecReturnCode.SUCCESS)
            {
                rejectReasons.Add(ItemRejectReason.ITEM_NOT_SUPPORTED);
                return null;
            }

            itemRequestInfo.DomainType = msg.DomainType;
            //copy item name buffer
            ByteBuffer byteBuffer = new ByteBuffer(itemRequestInfo.MsgKey.Name.Length);
            itemRequestInfo.MsgKey.Name.Copy(byteBuffer);
            itemRequestInfo.ItemName.Data(byteBuffer);
            int msgFlags = msg.Flags;
            if ((msgFlags & RequestMsgFlags.PRIVATE_STREAM) != 0)
            {
                itemRequestInfo.IsPrivateStreamRequest = true;
            }

            /* get IsStreamingRequest */
            if ((msgFlags & RequestMsgFlags.STREAMING) != 0)
            {
                itemRequestInfo.IsStreamingRequest = true;
            }

            /* get IncludeKeyInUpdates */
            if ((msgFlags & RequestMsgFlags.MSG_KEY_IN_UPDATES) != 0)
            {
                itemRequestInfo.IncludeKeyInUpdates = true;
            }

            //get item information
            itemRequestInfo.ItemInfo = m_ItemInfoWatchList.Get(channel, itemRequestInfo.ItemName,
                    itemRequestInfo.DomainType, itemRequestInfo.IsPrivateStreamRequest);
            if (itemRequestInfo.ItemInfo is null)
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

                if ((itemRequestInfo.ItemInfo.ItemData is null) && (itemRequestInfo.DomainType != (int)DomainType.SYMBOL_LIST))
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
            itemRequestInfo.ItemInfo!.InterestCount++;
            itemRequestInfo.StreamId = stream;

            //provide a refresh if one was requested.
            itemRequestInfo.ItemInfo.IsRefreshRequired = msg.CheckNoRefresh() ? false : true;

            return itemRequestInfo;
        }

        private void AddSymbolListItem(IChannel channel, ItemRequestInfo itemReqInfo)
        {
            int itemVacancy = 0;
            Buffer newItem = itemReqInfo.ItemName;
            bool foundVacancy = false;

            //TRI and RES-DS are added to our symbol list at initialization, and
            //are always present so they never need to be added again
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

            //find all consumers currently using the symbol list domain, and send
            //them updates
            foreach (ItemRequestInfo itemReqInfoL in m_ItemRequestWatchList)
            {
                if (itemReqInfoL.DomainType == (int)DomainType.SYMBOL_LIST)
                {
                    SendSLItemUpdates(itemReqInfoL.Channel!, itemReqInfo, SymbolListItems.SYMBOL_LIST_UPDATE_ADD, itemReqInfoL.StreamId);
                }
            }
        }

        private void SendSLItemUpdates(IChannel chnl, ItemRequestInfo itemReqInfo, int responseType, int streamId)
        {
            //get a buffer for the response
            ITransportBuffer msgBuf = chnl.GetBuffer(SymbolListItems.MAX_SYMBOL_LIST_SIZE, false, out Error? error);
            if (msgBuf is null)
            {
                Console.Write($"IChanel.GetBuffer(): Failed {error.Text}");
                return;
            }

            CodecReturnCode ret = m_SymbolListItemWatchList.EncodeResponse(chnl, itemReqInfo.ItemInfo!, msgBuf, streamId, true, ServiceId, itemReqInfo.IsStreamingRequest, 
                m_DictionaryHandler.Dictionary, responseType, out error);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeSymbolListResponse() failed");
            }

            if (m_ProviderSession.Write(chnl, msgBuf, out error) == TransportReturnCode.FAILURE)
                Console.WriteLine($"Error writing message: {error!.Text}");
        }

        public CodecReturnCode SendItemUpdates(IChannel channel, out Error? error)
        {
            error = null;
            foreach (ItemRequestInfo itemReqInfo in m_ItemRequestWatchList)
            {
                if (itemReqInfo.IsInUse && itemReqInfo.Channel == channel)
                {
                    CodecReturnCode ret = SendItemResponse(itemReqInfo, out error);
                    if (ret != CodecReturnCode.SUCCESS)
                        return ret;
                }
            }
            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode SendItemResponse(ItemRequestInfo itemReqInfo, out Error? error)
        {
            //market by price is handled separately due to multi-part refresh
            if (itemReqInfo.DomainType == (int)DomainType.MARKET_BY_PRICE)
            {
                return SendMBPItemResponse(itemReqInfo, out error);
            }

            //get a buffer for the response
            ITransportBuffer msgBuf = itemReqInfo.Channel!.GetBuffer(ITEM_MSG_SIZE, false, out error);
            if (msgBuf is null)
                return CodecReturnCode.FAILURE;

            //Encode the message with data appopriate for the domain
            switch (itemReqInfo.DomainType)
            {
                case (int)DomainType.MARKET_PRICE:
                    //encode market price response 
                    CodecReturnCode ret = m_MarketPriceItemWatchList.EncodeResponse(itemReqInfo.Channel, itemReqInfo.ItemInfo!, msgBuf, true, itemReqInfo.StreamId,
                        itemReqInfo.IsStreamingRequest, itemReqInfo.IsPrivateStreamRequest, ServiceId, m_DictionaryHandler.Dictionary, out error);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                    break;
                case (int)DomainType.MARKET_BY_ORDER:
                    //encode market by order response
                    ret = m_MarketByOrderItemWatchList.EncodeResponse(itemReqInfo.Channel, itemReqInfo.ItemInfo!, msgBuf, true, itemReqInfo.StreamId, itemReqInfo.IsStreamingRequest, 
                        itemReqInfo.IsPrivateStreamRequest, ServiceId, m_DictionaryHandler.Dictionary, out error);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                    break;
                case (int)DomainType.SYMBOL_LIST:
                    //encode symbol list response
                    //only encode refresh responses for the symbol list from this
                    //method. symbol list update responses are handled separately
                    if (itemReqInfo.ItemInfo!.IsRefreshRequired)
                    {
                        ret = m_SymbolListItemWatchList.EncodeResponse(itemReqInfo.Channel, itemReqInfo.ItemInfo!, msgBuf, itemReqInfo.StreamId, true, ServiceId, 
                            itemReqInfo.IsStreamingRequest, m_DictionaryHandler.Dictionary, SymbolListItems.SYMBOL_LIST_REFRESH, out error);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            return ret;
                        }
                    }
                    else
                    {
                        itemReqInfo.Channel.ReleaseBuffer(msgBuf, out error);
                        return CodecReturnCode.SUCCESS;
                    }
                    break;
                default:

                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"Received unhandled domain {itemReqInfo.DomainType} for item"
                    };

                    return CodecReturnCode.FAILURE;
            }

            //send item response
            return (CodecReturnCode)m_ProviderSession.Write(itemReqInfo.Channel, msgBuf, out error);
        }

        private CodecReturnCode SendMBPItemResponse(ItemRequestInfo itemReqInfo, out Error? error)
        {
            error = null;
            MarketByPriceItem mbpItem = new MarketByPriceItem();

            if (itemReqInfo.ItemInfo!.IsRefreshRequired)
            {
                mbpItem = (MarketByPriceItem)itemReqInfo.ItemInfo.ItemData!;
                for (int i = 0; i < MAX_REFRESH_PARTS; i++)
                {
                    //get a buffer for the response
                    ITransportBuffer msgBuf = itemReqInfo.Channel!.GetBuffer(ITEM_MSG_SIZE, false, out error);
                    if (msgBuf == null)
                        return CodecReturnCode.FAILURE;

                    if (msgBuf != null)
                    {
                        //encode market by price refresh 
                        CodecReturnCode ret = m_MarketByPriceItemWatchList.EncodeRefresh(itemReqInfo.Channel, itemReqInfo.ItemInfo, msgBuf, true, 
                            itemReqInfo.StreamId, itemReqInfo.IsStreamingRequest, itemReqInfo.IsPrivateStreamRequest, ServiceId,
                            m_DictionaryHandler.Dictionary, i, out error);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            return ret;
                        }

                        //send item response
                        ret = (CodecReturnCode)m_ProviderSession.Write(itemReqInfo.Channel, msgBuf, out error);
                        if (ret != CodecReturnCode.SUCCESS)
                            return ret;
                    }

                    //send an update between each part of the refresh
                    if (i < MAX_REFRESH_PARTS - 1)
                    {
                        mbpItem.PriceInfoList[0].ORDER_SIZE.Value(mbpItem.PriceInfoList[0].ORDER_SIZE.ToDouble() + i + 1, mbpItem.PriceInfoList[0].ORDER_SIZE.Hint); // change
                                                                                                                                                                     // order
                                                                                                                                                                     //size for update
                        mbpItem.PriceInfoList[1].ORDER_SIZE.Value(mbpItem.PriceInfoList[1].ORDER_SIZE.ToDouble() + i + 1, mbpItem.PriceInfoList[1].ORDER_SIZE.Hint); // change
                        mbpItem.PriceInfoList[2].ORDER_SIZE.Value(mbpItem.PriceInfoList[2].ORDER_SIZE.ToDouble() + i + 1, mbpItem.PriceInfoList[2].ORDER_SIZE.Hint); // change
                                                                                                                                                                     // order
                                                                                                                                                                     //get a buffer for the response
                        msgBuf = itemReqInfo.Channel.GetBuffer(ITEM_MSG_SIZE, false, out error);
                        if (msgBuf == null)
                            return CodecReturnCode.FAILURE;
                        //encode market by price update
                        CodecReturnCode ret = m_MarketByPriceItemWatchList.EncodeUpdate(itemReqInfo.Channel, itemReqInfo.ItemInfo, msgBuf, true, itemReqInfo.StreamId, 
                            itemReqInfo.IsStreamingRequest, itemReqInfo.IsPrivateStreamRequest, ServiceId, m_DictionaryHandler.Dictionary, out error);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            return ret;
                        }

                        //send item response
                        ret = (CodecReturnCode)m_ProviderSession.Write(itemReqInfo.Channel, msgBuf, out error);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            return ret;
                        }

                        mbpItem.PriceInfoList[0].ORDER_SIZE.Value(mbpItem.PriceInfoList[0].ORDER_SIZE.ToDouble() - (i + 1), mbpItem.PriceInfoList[0].ORDER_SIZE.Hint); // change
                                                                                                                                                                       // order
                                                                                                                                                                       // size for update
                        mbpItem.PriceInfoList[1].ORDER_SIZE.Value(mbpItem.PriceInfoList[1].ORDER_SIZE.ToDouble() - (i + 1), mbpItem.PriceInfoList[1].ORDER_SIZE.Hint); // change
                                                                                                                                                                       // order
                        mbpItem.PriceInfoList[2].ORDER_SIZE.Value(mbpItem.PriceInfoList[2].ORDER_SIZE.ToDouble() - (i + 1), mbpItem.PriceInfoList[2].ORDER_SIZE.Hint); // change
                    }
                }
            }
            else
            //update
            {
                //get a buffer for the response
                ITransportBuffer msgBuf = itemReqInfo.Channel!.GetBuffer(ITEM_MSG_SIZE, false, out error);
                if (msgBuf is null)
                    return CodecReturnCode.FAILURE;

                //encode market by price update
                CodecReturnCode ret = m_MarketByPriceItemWatchList.EncodeUpdate(itemReqInfo.Channel, itemReqInfo.ItemInfo, msgBuf, true, itemReqInfo.StreamId, 
                    itemReqInfo.IsStreamingRequest, itemReqInfo.IsPrivateStreamRequest, ServiceId, m_DictionaryHandler.Dictionary, out error);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                //send item response
                return (CodecReturnCode)m_ProviderSession.Write(itemReqInfo.Channel, msgBuf, out error);
            }

            return CodecReturnCode.SUCCESS;
        }

        private static CodecReturnCode CopyMsgKey(IMsgKey destKey, IMsgKey sourceKey)
        {
            destKey.Flags = sourceKey.Flags;
            destKey.NameType = sourceKey.NameType;
            if (sourceKey.CheckHasName() && sourceKey.Name != null)
            {
                destKey.Name.Data(new ByteBuffer(sourceKey.Name.Length));
                sourceKey.Name.Copy(destKey.Name);
            }
            destKey.ServiceId = sourceKey.ServiceId;
            destKey.Filter = sourceKey.Filter;
            destKey.Identifier = sourceKey.Identifier;
            destKey.AttribContainerType = sourceKey.AttribContainerType;
            if (sourceKey.CheckHasAttrib() && sourceKey.EncodedAttrib != null)
            {
                CodecReturnCode ret = sourceKey.EncodedAttrib.Copy(destKey.EncodedAttrib);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        private bool IsStreamInUse(IChannel chnl, int streamId, IMsgKey key)
        {
            bool streamInUse = false;

            foreach (ItemRequestInfo itemReqInfo in m_ItemRequestWatchList)
            {
                if (itemReqInfo.IsInUse &&
                        itemReqInfo.Channel == chnl &&
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

        private CodecReturnCode ProcessPost(IChannel chnl, Msg msg, DecodeIterator dIter, out Error? error)
        {
            error = null;
            LoginRequestInfo? loginRequestInfo;
            IPostMsg postMsg = msg;

            // get the login stream so that we can see if the post was an off-stream
            // post
            if ((loginRequestInfo = m_LoginHandler.FindLoginRequestInfo(chnl)) is null)
            {
                return SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, "Received a post message request from client before login", out error);
            }

            ItemInfo? itemInfo;
            // if the post is on the login stream, then it's an off-stream post
            if (loginRequestInfo.LoginRequest.StreamId == msg.StreamId)
            {
                // the msg key must be specified to provide the item name
                if (!postMsg.CheckHasMsgKey())
                {
                    return SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, "Received an off-stream post message request from client without a msgkey", out error);
                }
                Console.WriteLine($"Received an off-stream item post (item={postMsg.MsgKey.Name})");
                // look up the item name
                // for this example, we will treat an unknown item as an error
                // However, other providers may choose to add the item to their
                // cache
                if ((itemInfo = m_ItemInfoWatchList.Get(postMsg.MsgKey.Name, postMsg.DomainType, false)) is null)
                {
                    return SendAck(chnl, postMsg, NakCodes.SYMBOL_UNKNOWN, "Received an off-stream post message for an unknown item", out error);
                }
            }
            else
            {
                ItemRequestInfo? itemReqInfo = null;
                // the msgkey is not required for on-stream post
                // get the item request associated with this on-stream post
                if ((itemReqInfo = m_ItemRequestWatchList.Get(chnl, postMsg.StreamId)) is null)
                {
                    return SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, "Received an on-stream post message on a stream that does not have an item open", out error);
                }

                itemInfo = itemReqInfo.ItemInfo;
                Console.WriteLine($"Received an on-stream post for item= {itemInfo!.ItemName}");
            }

            // This is used to indicate whether an ACK message is sent.
            bool sendAck = false;
            CodecReturnCode ret;

            // if the post message contains another message, then use the
            // "contained" message as the update/refresh/status
            if (postMsg.ContainerType == DataTypes.MSG)
            {
                m_NestedMsg.Clear();
                ret = m_NestedMsg.Decode(dIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"Unable to decode msg"
                    };

                    return ret;
                }
                switch (m_NestedMsg.MsgClass)
                {
                    case MsgClasses.REFRESH:
                        m_NestedMsg.MsgClass = MsgClasses.REFRESH;
                        int flags = m_NestedMsg.Flags;
                        flags |= RefreshMsgFlags.HAS_POST_USER_INFO;
                        flags &= ~RefreshMsgFlags.SOLICITED;
                        m_NestedMsg.Flags = flags;

                        m_NestedMsg.PostUserInfo.UserAddr  = postMsg.PostUserInfo.UserAddr;
                        m_NestedMsg.PostUserInfo.UserId = postMsg.PostUserInfo.UserId;
                        if (UpdateItemInfoFromPost(itemInfo, m_NestedMsg, dIter, out error) != CodecReturnCode.SUCCESS)
                        {
                            ret = SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, error!.Text, out error);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }

                            sendAck = true;
                        }

                        break;

                    case MsgClasses.UPDATE:
                        m_NestedMsg.MsgClass = MsgClasses.UPDATE;
                        m_NestedMsg.Flags |= UpdateMsgFlags.HAS_POST_USER_INFO;
                        m_NestedMsg.PostUserInfo.UserAddr = postMsg.PostUserInfo.UserAddr;
                        m_NestedMsg.PostUserInfo.UserId = postMsg.PostUserInfo.UserId;
                        if (UpdateItemInfoFromPost(itemInfo, m_NestedMsg, dIter, out error) != CodecReturnCode.SUCCESS)
                        {
                            ret = SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, error!.Text, out error);
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return ret;
                            }

                            sendAck = true;
                        }
                        break;

                    case MsgClasses.STATUS:
                        m_NestedMsg.MsgClass =MsgClasses.STATUS;
                        m_NestedMsg.Flags |= StatusMsgFlags.HAS_POST_USER_INFO;
                        m_NestedMsg.PostUserInfo.UserAddr = postMsg.PostUserInfo.UserAddr;
                        m_NestedMsg.PostUserInfo.UserId = postMsg.PostUserInfo.UserId;
                        if (m_NestedMsg.CheckHasState() && m_NestedMsg.State.StreamState() == StreamStates.CLOSED)
                        {
                            // check if the user has the rights to send a post that
                            // closes an item
                            if (postMsg.CheckHasPostUserRights() || postMsg.PostUserRights == 0)
                            {
                                ret = SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, "client has insufficient rights to close/delete an item", out error);
                                if (ret != CodecReturnCode.SUCCESS)
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
                m_UpdateMsg.Clear();
                m_UpdateMsg.MsgClass = MsgClasses.UPDATE;
                m_UpdateMsg.DomainType = postMsg.DomainType;
                m_UpdateMsg.ContainerType = postMsg.ContainerType;
                if (msg.EncodedDataBody != null && msg.EncodedDataBody.Length > 0)
                    m_UpdateMsg.EncodedDataBody =msg.EncodedDataBody;
                m_UpdateMsg.Flags = UpdateMsgFlags.HAS_POST_USER_INFO;
                m_UpdateMsg.PostUserInfo.UserAddr = postMsg.PostUserInfo.UserAddr;
                m_UpdateMsg.PostUserInfo.UserId = postMsg.PostUserInfo.UserId;
                if (postMsg.CheckHasMsgKey())
                {
                    m_UpdateMsg.Flags |= UpdateMsgFlags.HAS_MSG_KEY;
                    m_UpdateMsg.MsgKey.Copy(postMsg.MsgKey);
                }

                if (UpdateItemInfoFromPost(itemInfo, msg, dIter, out error) != CodecReturnCode.SUCCESS)
                {
                    ret = SendAck(chnl, postMsg, NakCodes.INVALID_CONTENT, error!.Text, out error);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    sendAck = true;
                }
            }

            if (sendAck == false)
            {
                ret = SendAck(chnl, postMsg, NakCodes.NONE, null, out error);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            // send the post to all public streams with this item open
            foreach (ItemRequestInfo itemReqInfoL in m_ItemRequestWatchList)
            {
                if (itemReqInfoL.ItemInfo == itemInfo)
                {
                    m_EncodeIter.Clear();
                    ITransportBuffer sendBuf = itemReqInfoL.Channel!.GetBuffer(POST_MSG_SIZE, false, out error);
                    if (sendBuf == null)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    ret = m_EncodeIter.SetBufferAndRWFVersion(sendBuf, itemReqInfoL.Channel.MajorVersion, itemReqInfoL.Channel.MinorVersion);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        error = new Error
                        {
                            ErrorId = TransportReturnCode.FAILURE,
                            Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                        };

                        return CodecReturnCode.FAILURE;
                    }

                    if (postMsg.ContainerType == DataTypes.MSG)
                    {
                        // send the contained/embedded message if there was one.
                        m_NestedMsg.StreamId = itemReqInfoL.StreamId;
                        if (m_NestedMsg.MsgClass == MsgClasses.REFRESH)
                        {
                            m_NestedMsg.ApplyHasMsgKey();
                        }
                        ret = m_NestedMsg.Encode(m_EncodeIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {

                            //error.text("nestedMsg.encode() failed");
                            return CodecReturnCode.FAILURE;
                        }
                        ret = (CodecReturnCode)m_ProviderSession.Write(itemReqInfoL.Channel, sendBuf, out error);
                        if (ret != CodecReturnCode.SUCCESS)
                            return CodecReturnCode.FAILURE;

                        // check if its a status close and close any open streams if
                        // it is
                        if (m_NestedMsg.MsgClass == MsgClasses.STATUS && m_NestedMsg.CheckHasState() && m_NestedMsg.State.StreamState() == StreamStates.CLOSED)
                            CloseStream(itemReqInfoL.Channel, m_NestedMsg.StreamId);
                    }
                    else
                    {
                        // send an update message if the post contained data
                        m_UpdateMsg.StreamId = itemReqInfoL.StreamId;
                        ret = m_UpdateMsg.Encode(m_EncodeIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"nestedMsg.Encode() failed"
                            };

                            return CodecReturnCode.FAILURE;
                        }
                        ret = (CodecReturnCode)m_ProviderSession.Write(itemReqInfoL.Channel, sendBuf, out error);
                        if (ret != CodecReturnCode.SUCCESS)
                            return CodecReturnCode.FAILURE;
                    }
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode UpdateItemInfoFromPost(ItemInfo itemInfo, Msg msg, DecodeIterator dIter, out Error? error)
        {
            error = null;
            CodecReturnCode ret;

            switch (itemInfo.DomainType)
            {
                case (int)DomainType.MARKET_PRICE:
                    ret = m_MarketPriceItemWatchList.UpdateFieldsFromPost((MarketPriceItem)itemInfo.ItemData!, dIter, out error);
                    break;

                case (int)DomainType.MARKET_BY_ORDER:
                case (int)DomainType.MARKET_BY_PRICE:
                default:

                    error = new Error
                    {
                        ErrorId =TransportReturnCode.FAILURE,
                        Text = $"Unsupported domain {itemInfo.DomainType} in post message update/refresh"
                    };

                    ret = CodecReturnCode.FAILURE;
                    break;
            }

            return ret;
        }

        /// <summary>
        /// Closes all item requests for the closed channel.
        /// </summary>
        /// <param name="channel">channel for which channel close has received.</param>
        public void CloseRequests(IChannel channel)
        {
            //find original item request information associated with channel
            foreach (ItemRequestInfo itemRequestInfoL in m_ItemRequestWatchList)
            {
                if (itemRequestInfoL.Channel == channel && itemRequestInfoL.IsInUse)
                {
                    FreeItemReqInfo(itemRequestInfoL);
                }
            }
        }

        /// <summary>
        /// Updates item information for all items in the watch list.
        /// </summary>
        public void UpdateItemInfo()
        {
            m_ItemInfoWatchList.Update();
        }
    }
}
