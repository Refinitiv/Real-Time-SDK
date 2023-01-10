/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Example.Common;
using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using ItemInfo = LSEG.Eta.PerfTools.Common.ItemInfo;
using ProviderSession = LSEG.Eta.PerfTools.Common.ProviderSession;

namespace LSEG.Eta.PerfTools.ProvPerf
{
    /// <summary>
    /// Implementation of handling item requests for the ProvPerf application.
    /// </summary>
    public class ItemRequestHandler
    {
        private IProvMarketPriceDecoder m_MarketPriceDecoder;
        private MarketPriceStatus m_ItemStatusMessage;
        private ItemAttributes m_ItemAttributes;
        private EncodeIterator m_EncIter;
        private Msg m_TmpMsg;

        public ItemRequestHandler()
        {
            m_EncIter = new EncodeIterator();
            m_ItemStatusMessage = new MarketPriceStatus();
            m_ItemAttributes = new ItemAttributes();
            m_TmpMsg = new Msg();
            m_MarketPriceDecoder = new IProvMarketPriceDecoder();
        }

        /// <summary>
        /// Processes item request message
        /// </summary>
        /// <param name="providerThread">Provider thread that received the request</param>
        /// <param name="providerSession">Provider channel session</param>
        /// <param name="msg">request message</param>
        /// <param name="openLimit">item count limit</param>
        /// <param name="serviceId">service id</param>
        /// <param name="dirQos">Qos provided by source directory</param>
        /// <param name="decodeIter">Decode iterator containing received message buffer</param>
        /// <param name="error">in case of error, this object is populated with error information</param>
        /// <returns><see cref="TransportReturnCode"/> indicating the status of the operation</returns>
        public TransportReturnCode ProcessMsg(ProviderThread providerThread, ProviderSession providerSession, Msg msg, int openLimit, int serviceId, Qos dirQos, DecodeIterator decodeIter, out Error? error)
        {
            error = null;
            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    providerThread.ItemRequestCount.Increment();

                    // get key
                    IMsgKey msgKey = msg.MsgKey;

                    // check if item count reached
                    if (providerSession.OpenItemCount >= openLimit)
                    {
                        return SendRequestReject(providerThread, providerSession, msg, ItemRejectReason.ITEM_COUNT_REACHED, out error);
                    }
                    // check if service id correct
                    if (msgKey.ServiceId != serviceId)
                    {
                        return SendRequestReject(providerThread, providerSession, msg, ItemRejectReason.INVALID_SERVICE_ID, out error);
                    }
                    // check if QoS supported
                    if (msg.CheckHasQos() && msg.CheckHasWorstQos())
                    {
                        if (!dirQos.IsInRange(msg.Qos, msg.WorstQos))
                        {
                            return SendRequestReject(providerThread, providerSession, msg, ItemRejectReason.QOS_NOT_SUPPORTED, out error);
                        }
                    }
                    else if (msg.CheckHasQos())
                    {
                        if (!dirQos.Equals(msg.Qos))
                        {
                            return SendRequestReject(providerThread, providerSession, msg, ItemRejectReason.QOS_NOT_SUPPORTED, out error);
                        }
                    }

                    // check if item already opened with exact same key and domain.
                    // If we find one, check the StreamId.
                    // If the streamId matches, it is a reissue.
                    // If the streamId does not match, reject the redundant request.
                    m_ItemAttributes.DomainType = msg.DomainType;
                    m_ItemAttributes.MsgKey = msgKey;
                    ItemInfo? itemInfo = FindAlreadyOpenedItem(providerSession, m_ItemAttributes);
                    if (itemInfo != null && itemInfo.StreamId != msg.StreamId)
                    {
                        return SendRequestReject(providerThread, providerSession, msg, ItemRejectReason.ITEM_ALREADY_OPENED, out error);
                    }

                    if (IsStreamInUse(providerSession, msg.StreamId, m_ItemAttributes))
                    {
                        return SendRequestReject(providerThread, providerSession, msg, ItemRejectReason.STREAM_ALREADY_IN_USE, out error);
                    }

                    if (itemInfo == null)
                    {
                        // New request

                        // get item info structure
                        ItemAttributes itemAttributes = new ItemAttributes();
                        itemAttributes.DomainType = msg.DomainType;
                        itemAttributes.MsgKey = new MsgKey();

                        // MsgKey.copy() - shallow copy for name and attrib
                        msgKey.Copy(itemAttributes.MsgKey);

                        if (msgKey.CheckHasName())
                        {
                            // deep copy item name buffer
                            ByteBuffer nameBytes = new ByteBuffer(msgKey.Name.Length);
                            msgKey.Name.Copy(nameBytes);
                            itemAttributes.MsgKey.Name.Data(nameBytes);
                        }

                        if (msgKey.CheckHasAttrib())
                        {
                            // deep copy attrib buffer
                            ByteBuffer attribBytes = new ByteBuffer(msgKey.EncodedAttrib.Length);
                            msgKey.EncodedAttrib.Copy(attribBytes);
                            itemAttributes.MsgKey.EncodedAttrib.Data(attribBytes);
                        }

                        itemInfo = providerSession.CreateItemInfo(itemAttributes, msg.StreamId, out error);

                        if (itemInfo == null)
                        {
                            error = new Error()
                            {
                                Text = "Failed to create ItemInfo instance",
                                ErrorId = TransportReturnCode.FAILURE
                            };
                            return TransportReturnCode.FAILURE;
                        }

                        // get StreamId
                        itemInfo.StreamId = msg.StreamId;
                        itemInfo.ItemFlags = itemInfo.ItemFlags | (int)ItemFlags.IS_SOLICITED;
                        providerSession.ItemAttributesTable.Add(itemAttributes, itemInfo);
                        providerSession.ItemStreamIdTable.Add(msg.StreamId, itemInfo);
                    }
                    else
                    {
                        // else it was a reissue
                        if (!msg.CheckNoRefresh())
                        {
                            // Move item back to refresh queue.
                            providerSession.RefreshItemList.Add(itemInfo);
                        }
                    }

                    // get IsStreamingRequest
                    if (msg.CheckStreaming())
                    {
                        itemInfo.ItemFlags = itemInfo.ItemFlags | (int)ItemFlags.IS_STREAMING_REQ;
                    }

                    // check if the request is for a private stream
                    if (msg.CheckPrivateStream())
                    {
                        itemInfo.ItemFlags = itemInfo.ItemFlags | (int)ItemFlags.IS_PRIVATE;
                    }
                    break;
                case MsgClasses.POST:
                    providerThread.PostMsgCount.Increment();
                    return ReflectPostMsg(providerThread, providerSession, decodeIter, msg, out error);
                case MsgClasses.GENERIC:
                    if (providerThread.ProvThreadInfo!.Stats.FirstGenMsgRecvTime == 0)
                        providerThread.ProvThreadInfo.Stats.FirstGenMsgRecvTime = (long)GetTime.GetNanoseconds();
                    providerThread.ProvThreadInfo.Stats.GenMsgRecvCount.Increment();
                    return ProcessGenMsg(providerThread, decodeIter, providerSession, msg);
                case MsgClasses.CLOSE:
                    // close item stream
                    providerThread.CloseMsgCount.Increment();
                    providerSession.CloseItemStream(msg.StreamId);
                    break;
                default:
                    Console.WriteLine($"\nReceived Unhandled Item Msg Class: {msg.MsgClass}\n");
                    break;
            }

            return TransportReturnCode.SUCCESS;
        }

        public TransportReturnCode SendRequestReject(ProviderThread providerThread, ProviderSession providerSession, Msg msg, ItemRejectReason reason, out Error? error)
        {
            TransportReturnCode transportReturnCode = providerThread.GetItemMsgBuffer(providerSession, 128, out error);
            if (transportReturnCode < TransportReturnCode.SUCCESS)
            {
                return TransportReturnCode.FAILURE;
            }

            CodecReturnCode codecReturnCode = EncodeItemReject(providerSession, msg, reason, out error);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                return TransportReturnCode.FAILURE;
            }
            Console.WriteLine($"Rejecting Item Request with streamId={msg.StreamId} and domain {DomainTypes.ToString(msg.DomainType)}.  Reason: {reason.GetAsString()}");

            return providerThread.SendItemMsgBuffer(providerSession, true, out error);
        }

        private CodecReturnCode EncodeItemReject(ProviderSession providerSession, Msg msg, ItemRejectReason reason, out Error? error)
        {
            // encode request reject status
            m_ItemStatusMessage.Clear();
            m_ItemStatusMessage.HasState = true;
            m_ItemStatusMessage.State.StreamState(StreamStates.CLOSED_RECOVER);
            m_ItemStatusMessage.State.DataState(DataStates.SUSPECT);

            // encode request reject status
            switch (reason)
            {
                case ItemRejectReason.ITEM_COUNT_REACHED:
                    m_ItemStatusMessage.State.Code(StateCodes.TOO_MANY_ITEMS);
                    m_ItemStatusMessage.State.Text().Data($"Item request rejected for stream id {msg.StreamId} - item count reached for this channel");
                    break;
                case ItemRejectReason.INVALID_SERVICE_ID:
                    m_ItemStatusMessage.State.Code(StateCodes.USAGE_ERROR);
                    m_ItemStatusMessage.State.StreamState(StreamStates.CLOSED);
                    m_ItemStatusMessage.State.Text().Data($"Item request rejected for stream id {msg.StreamId} - service id invalid");
                    break;
                case ItemRejectReason.QOS_NOT_SUPPORTED:
                    m_ItemStatusMessage.State.Code(StateCodes.USAGE_ERROR);
                    m_ItemStatusMessage.State.StreamState(StreamStates.CLOSED);
                    m_ItemStatusMessage.State.Text().Data($"Item request rejected for stream id {msg.StreamId} - QoS not supported");
                    break;
                case ItemRejectReason.ITEM_ALREADY_OPENED:
                    m_ItemStatusMessage.State.Code(StateCodes.ALREADY_OPEN);
                    m_ItemStatusMessage.State.StreamState(StreamStates.CLOSED);
                    m_ItemStatusMessage.State.Text().Data($"Item request rejected for stream id {msg.StreamId} - item already open with exact same key on another stream");
                    break;
                case ItemRejectReason.STREAM_ALREADY_IN_USE:
                    m_ItemStatusMessage.State.Code(StateCodes.USAGE_ERROR);
                    m_ItemStatusMessage.State.StreamState(StreamStates.CLOSED);
                    m_ItemStatusMessage.State.Text().Data($"Item request rejected for stream id {msg.StreamId} - stream already in use with a different key");
                    break;
                case ItemRejectReason.KEY_ENC_ATTRIB_NOT_SUPPORTED:
                    m_ItemStatusMessage.State.Code(StateCodes.USAGE_ERROR);
                    m_ItemStatusMessage.State.StreamState(StreamStates.CLOSED);
                    m_ItemStatusMessage.State.Text().Data($"Item request rejected for stream id {msg.StreamId} - this provider does not support key attribute information");
                    break;
                case ItemRejectReason.ITEM_NOT_SUPPORTED:
                    m_ItemStatusMessage.State.Code(StateCodes.USAGE_ERROR);
                    m_ItemStatusMessage.State.StreamState(StreamStates.CLOSED);
                    m_ItemStatusMessage.State.Text().Data($"Item request rejected for stream id {msg.StreamId} - item not supported");
                    break;
                case ItemRejectReason.PRIVATE_STREAM_REDIRECT:
                    m_ItemStatusMessage.PrivateStream = true;
                    m_ItemStatusMessage.State.Code(StateCodes.NONE);
                    m_ItemStatusMessage.State.StreamState(StreamStates.REDIRECTED);
                    m_ItemStatusMessage.State.Text().Data($"Standard stream redirect to private for stream id {msg.StreamId} - this item must be requested via private stream");
                    break;
                case ItemRejectReason.PRIVATE_STREAM_MISMATCH:
                    m_ItemStatusMessage.PrivateStream = true;
                    m_ItemStatusMessage.State.Code(StateCodes.USAGE_ERROR);
                    m_ItemStatusMessage.State.StreamState(StreamStates.CLOSED);
                    m_ItemStatusMessage.State.Text().Data($"Rejected request for stream id {msg.StreamId} - reissue via batch request is not allowed");
                    break;
                default:
                    break;
            }

            m_EncIter.Clear();
            m_ItemStatusMessage.StreamId = msg.StreamId;
            m_ItemStatusMessage.DomainType = msg.DomainType;
            CodecReturnCode ret = m_EncIter.SetBufferAndRWFVersion(providerSession.WritingBuffer, providerSession.ClientChannelInfo!.Channel!.MajorVersion, providerSession.ClientChannelInfo.Channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };
                return ret;
            }

            ret = m_ItemStatusMessage.Encode(m_EncIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"MarketPriceStatus.Encode() failed with return code: {ret.GetAsString()}"
                };
                return ret;
            }

            error = null;
            return CodecReturnCode.SUCCESS;
        }

        private bool IsStreamInUse(ProviderSession providerSession, int streamId, ItemAttributes attributes)
        {
            if (!providerSession.ItemAttributesTable.ContainsKey(attributes))
            {
                return false;
            }
            ItemInfo itemInfo = providerSession.ItemAttributesTable[attributes];
            if (itemInfo == null)
                return false;

            return itemInfo.StreamId == streamId && itemInfo.Attributes.MsgKey!.Equals(attributes.MsgKey);
        }

        private ItemInfo? FindAlreadyOpenedItem(ProviderSession providerSession, ItemAttributes attributes)
        {
            return providerSession.ItemAttributesTable.ContainsKey(attributes) ? providerSession.ItemAttributesTable[attributes] : null;
        }

        /// <summary>
        /// Decode post message received, sends updated market data message 
        /// to the connected clients subscribed to the item.
        /// </summary>
        /// <param name="providerThread">current provider thread</param>
        /// <param name="providerSession">current provider session</param>
        /// <param name="decodeIter"><see cref="DecodeIterator"/> instance used to decode the message</param>
        /// <param name="postMsg">partially decoded post message</param>
        /// <param name="error"><see cref="Error"/> instance that carries additional error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the operation</returns>
        private TransportReturnCode ReflectPostMsg(ProviderThread providerThread, ProviderSession providerSession, DecodeIterator decodeIter, IPostMsg postMsg, out Error error)
        {
            CodecReturnCode codecReturnCode;
            switch (postMsg.ContainerType)
            {
                case DataTypes.MSG:
                    m_TmpMsg.Clear();
                    codecReturnCode = m_TmpMsg.Decode(decodeIter);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"Failed to decode post message, return code: {codecReturnCode.GetAsString()}"
                        };
                        return TransportReturnCode.FAILURE;
                    }                       
                    break;
                default:
                    // It's a container (e.g. field list). Add an update header for reflecting
                    m_TmpMsg.Clear();
                    m_TmpMsg.MsgClass = MsgClasses.UPDATE;
                    m_TmpMsg.ContainerType = postMsg.ContainerType;
                    m_TmpMsg.DomainType = postMsg.DomainType;
                    postMsg.EncodedDataBody.Copy(m_TmpMsg.EncodedDataBody);
                    break;
            }

            // get a buffer for the response
            TransportReturnCode transportReturnCode = providerThread.GetItemMsgBuffer(providerSession, 128 + m_TmpMsg.EncodedDataBody.Length, out error!);
            if (transportReturnCode < TransportReturnCode.SUCCESS)
            {
                return TransportReturnCode.FAILURE;
            }
            int len = providerSession.WritingBuffer!.Data.Contents.Length;
            // Add the post user info from the post message to the nested message
            // and re-encode.
            m_EncIter.Clear();
            codecReturnCode = m_EncIter.SetBufferAndRWFVersion(providerSession.WritingBuffer, providerSession.ClientChannelInfo!.Channel!.MajorVersion, providerSession.ClientChannelInfo.Channel.MinorVersion);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                Console.Error.WriteLine($"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {codecReturnCode.GetAsString()}");
                error = new Error()
                {
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {codecReturnCode.GetAsString()}"
                };
                return TransportReturnCode.FAILURE;
            }

            // Add stream ID of PostMsg to nested message.
            m_TmpMsg.StreamId = postMsg.StreamId;

            // Add PostUserInfo of PostMsg to nested message.
            switch (m_TmpMsg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    m_TmpMsg.PostUserInfo.UserAddr = postMsg.PostUserInfo.UserAddr;
                    m_TmpMsg.PostUserInfo.UserId = postMsg.PostUserInfo.UserId;
                    m_TmpMsg.ApplyHasPostUserInfo();
                    m_TmpMsg.Flags &= ~RefreshMsgFlags.SOLICITED;
                    break;
                case MsgClasses.UPDATE:
                case MsgClasses.STATUS:
                    m_TmpMsg.PostUserInfo.UserAddr = postMsg.PostUserInfo.UserAddr;
                    m_TmpMsg.PostUserInfo.UserId = postMsg.PostUserInfo.UserId;
                    m_TmpMsg.ApplyHasPostUserInfo();
                    break;
                default:
                    Console.Error.WriteLine($"Error: Unhandled message class in post: {MsgClasses.ToString(m_TmpMsg.MsgClass)} ({m_TmpMsg.MsgClass})\n");
                    error = new Error()
                    {
                        Text = $"Error: Unhandled message class in post: {MsgClasses.ToString(m_TmpMsg.MsgClass)} ({m_TmpMsg.MsgClass})\n"
                    };
                    return TransportReturnCode.FAILURE;
            }

            // Other header members & data body should be properly set, so
            // re-encode.
            if ((codecReturnCode = m_TmpMsg.Encode(m_EncIter)) != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"Failed encoding message, return code: {codecReturnCode.GetAsString()}"
                };
                return TransportReturnCode.FAILURE;
            }

            return providerThread.SendItemMsgBuffer(providerSession, true, out error!);
        }

        TransportReturnCode ProcessGenMsg(ProviderThread providerThread, DecodeIterator decodeIter, ProviderSession providerSession, IGenericMsg genMsg)
        {
            CodecReturnCode codecReturnCode;
            codecReturnCode = DecodePayload(providerThread, decodeIter, providerSession, genMsg);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
                return TransportReturnCode.FAILURE;

            return TransportReturnCode.SUCCESS;
        }

        CodecReturnCode DecodePayload(ProviderThread providerThread, DecodeIterator decodeIter, ProviderSession providerSession, IGenericMsg genMsg)
        {
            switch (genMsg.DomainType)
            {
                case (int)DomainType.MARKET_PRICE:
                    return m_MarketPriceDecoder.DecodeUpdate(decodeIter, (Msg)genMsg, providerThread.ProvThreadInfo!);
                default:
                    break;
            }
            return CodecReturnCode.SUCCESS;
        }
    }
}
