/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using LSEG.Eta.Transports;

using Buffer = LSEG.Eta.Codec.Buffer;
using Msg = LSEG.Eta.Codec.Msg;
using System.Collections.Generic;
using Enum = LSEG.Eta.Codec.Enum;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{
    public enum ItemRejectReason
    {
        NONE,
        ITEM_COUNT_REACHED,
        INVALID_SERVICE_ID,
        QOS_NOT_SUPPORTED,
        ITEM_ALREADY_OPENED,
        STREAM_ALREADY_IN_USE,
        KEY_ENC_ATTRIB_NOT_SUPPORTED,
        PRIVATE_STREAM_REDIRECT,
        PRIVATE_STREAM_MISMATCH,
        ITEM_NOT_SUPPORTED,
        REQUEST_DECODE_ERROR,
        BATCH_ITEM_REISSUE,
        DOMAIN_NOT_SUPPORTED
    }

    internal class MarketItemHandler
    {
        private const int REJECT_MSG_SIZE = 1024;
        private const int MSG_BUFFER_SIZE = 1024;
        private Buffer m_TriItemName = new Buffer();
        private Buffer m_PrivateStreamItemName = new Buffer();
        private Buffer m_slNameBuf = new Buffer();
        private Buffer m_batchReqName = new Buffer();

        private LoginHandler m_LoginHandler;

        private IAckMsg m_AckMsg = new Eta.Codec.Msg();
        private IUpdateMsg m_UpdateMsg = new Eta.Codec.Msg();
        private IStatusMsg m_StatusMsg = new Eta.Codec.Msg();
        private Qos m_ProviderQos = new Qos();

        public int ServiceId { get; set; }

        DataDictionary m_DataDictionary;

        // Decoding
        private DecodeIterator m_DecodeIterator = new ();
        private Eta.Codec.ElementList m_ElementList = new ();
        private Eta.Codec.ElementEntry m_ElementEntry = new ();
        private Int m_Int = new ();
        private Buffer m_Buffer = new();
        private ulong m_ViewType;
        private Eta.Codec.Array m_Array = new Eta.Codec.Array();
        private Eta.Codec.ArrayEntry m_ArrayEntry = new Eta.Codec.ArrayEntry();

        public HashSet<long> ViewFids = new ();
        public HashSet<string> ViewNames = new ();

        // Message Encoding
        private EncodeIterator m_EncodeIterator = new EncodeIterator();
        private IRefreshMsg m_RefreshMsg = new Eta.Codec.Msg();
        private IGenericMsg m_GenericMsg = new Eta.Codec.Msg();
        private Real m_Real = new Real();
        private UInt m_UInt = new UInt();
        private Eta.Codec.FieldList m_FieldList = new Eta.Codec.FieldList();
        private Eta.Codec.FieldEntry m_FieldEntry = new Eta.Codec.FieldEntry();
        private Buffer m_encodedBuffer = new Buffer();
        private ByteBuffer m_MsgByteBuffer = new ByteBuffer(MSG_BUFFER_SIZE);

        private int m_numberOfMarketItems = 10;
        public int MarketPriceItemIndex = 0;
        public List<MarketPriceItem> MarketPriceItems = new ();


        private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();
        private ProviderSessionOptions m_ProviderSessionOptions;

        public MarketItemHandler(LoginHandler loginHandler, ProviderSessionOptions providerSessionOptions)
        {
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

            m_ProviderSessionOptions = providerSessionOptions;

            ServiceId = ProviderTest.DefaultService.ServiceId;

            m_DataDictionary = ProviderTest.DataDictionary;

            m_encodedBuffer.Data(m_MsgByteBuffer);

            for(int i = 0; i < m_numberOfMarketItems; i++)
            {
                MarketPriceItems.Add(new MarketPriceItem());
            }
        }

        public ReactorReturnCode ProcessRequest(ReactorChannel chnl, Eta.Codec.Msg msg, DecodeIterator dIter)
        {
            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    if (!msg.MsgKey.CheckHasServiceId() || msg.MsgKey.ServiceId != ServiceId)
                    {
                        return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.INVALID_SERVICE_ID, false);
                    }

                    //check if QoS supported 
                    if ((msg.Flags & (int)RequestMsgFlags.HAS_WORST_QOS) != 0 && (msg.Flags & (int)RequestMsgFlags.HAS_QOS) != 0)
                    {
                        if (!msg.Qos.IsInRange(m_ProviderQos, msg.WorstQos))
                        {
                            return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.QOS_NOT_SUPPORTED, false);
                        }
                    }
                    else if ((msg.Flags & (int)RequestMsgFlags.HAS_QOS) != 0)
                    {
                        if (!msg.Qos.Equals(m_ProviderQos))
                        {
                            return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.QOS_NOT_SUPPORTED, false);
                        }
                    }

                    //check for unsupported key attribute information
                    if (msg.MsgKey.CheckHasAttrib())
                    {
                        return SendItemRequestReject(chnl, msg.StreamId, msg.DomainType, ItemRejectReason.KEY_ENC_ATTRIB_NOT_SUPPORTED, false);
                    }

                    if (msg.CheckHasBatch())
                    {
                        return ProcessBatchRequest(chnl, msg, dIter, msg.CheckPrivateStream());
                    }
                    else
                    {
                        return ProcessSingleItemRequest(chnl, msg, dIter, msg.CheckPrivateStream());
                    }
                case MsgClasses.CLOSE:
                    Console.WriteLine($"Received Item close for streamId {msg.StreamId}");
                    //CloseStream(chnl.Channel!, msg.StreamId);
                    break;
                case MsgClasses.POST:
                    return ProcessPost(chnl, msg, dIter);
                default:
                    return ReactorReturnCode.FAILURE;
            }

            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode ProcessPost(ReactorChannel chnl, Eta.Codec.Msg msg, DecodeIterator dIter)
        {
            IPostMsg postMsg = msg;

            if (postMsg.CheckAck())
            {
                m_AckMsg.Clear();
                m_AckMsg.MsgClass = MsgClasses.ACK;
                m_AckMsg.StreamId = msg.StreamId;
                m_AckMsg.DomainType = msg.DomainType;
                m_AckMsg.ContainerType = DataTypes.MSG;
                m_AckMsg.ApplyHasMsgKey();
                m_AckMsg.MsgKey.ApplyHasName();
                m_AckMsg.MsgKey.Name = msg.MsgKey.Name;
                m_AckMsg.AckId = msg.PostId;

                ByteBuffer byteBuffer = new(msg.EncodedDataBody.Length);

                byteBuffer.Put(msg.EncodedDataBody.Data().Contents, msg.EncodedDataBody.Position,
                    msg.EncodedDataBody.Length); byteBuffer.Flip();

                Buffer dataBodyBuf = new();
                dataBodyBuf.Data(byteBuffer);

                m_AckMsg.EncodedDataBody = dataBodyBuf;

                m_SubmitOptions.Clear();
                return chnl.Submit(m_AckMsg, m_SubmitOptions, out _);
            }

            return ReactorReturnCode.SUCCESS;
        }

        private void CloseStream(IChannel channel, int streamId)
        {
            throw new NotImplementedException();
        }

        public ReactorReturnCode ProcessSingleItemRequest(ReactorChannel chnl, Eta.Codec.Msg msg, DecodeIterator dIter, bool isPrivateStream)
        {
            if (!m_ProviderSessionOptions.SendMarketDataItemResp)
            {
                return ReactorReturnCode.SUCCESS;
            }

            IRequestMsg requestMsg = msg;

            if(requestMsg.CheckHasView() && requestMsg.ContainerType == DataTypes.ELEMENT_LIST)
            {
                m_DecodeIterator.Clear();
                CodecReturnCode ret = m_DecodeIterator.SetBufferAndRWFVersion(requestMsg.EncodedDataBody,
                    chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

                Assert.Equal(CodecReturnCode.SUCCESS, ret);

                ret = m_ElementList.Decode(m_DecodeIterator,null);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);

                m_ViewType = 0;

                while (m_ElementEntry.Decode(m_DecodeIterator) != CodecReturnCode.END_OF_CONTAINER)
                {
                    if (m_ElementEntry.Name.ToString().Equals(Rdm.EmaRdm.ENAME_VIEW_TYPE))
                    {
                        UInt uInt = new UInt();

                        Assert.Equal(CodecReturnCode.SUCCESS, uInt.Decode(m_DecodeIterator));

                        m_ViewType = (ulong)uInt.ToLong();
                    }
                    else if (m_ElementEntry.Name.ToString().Equals(Rdm.EmaRdm.ENAME_VIEW_DATA))
                    {
                        ret = m_Array.Decode(m_DecodeIterator);

                        //Assert.Equal(CodecReturnCode.SUCCESS, m_Array.Decode(m_DecodeIterator));

                        Assert.Equal(DataTypes.INT, m_Array.PrimitiveType);

                        ViewFids.Clear();
                        ViewNames.Clear();

                        while (m_ArrayEntry.Decode(m_DecodeIterator) != CodecReturnCode.END_OF_CONTAINER)
                        {
                            
                            if (m_Array.PrimitiveType == DataTypes.INT)
                            {
                                m_Int.Clear();

                                Assert.Equal(CodecReturnCode.SUCCESS, m_Int.Decode(m_DecodeIterator));

                                ViewFids.Add(m_Int.ToLong());
                            }
                            else if (m_Array.PrimitiveType == DataTypes.ASCII_STRING)
                            {
                                m_Buffer.Clear();

                                Assert.Equal(CodecReturnCode.SUCCESS, m_Buffer.Decode(m_DecodeIterator));

                                ViewNames.Add(m_Buffer.ToString());

                            }
                        }

                    }
                }
            } // End checking the View data

            if (!msg.CheckNoRefresh())
            {
                ReactorReturnCode ret = SendItemResponse(chnl, msg);
                Thread.Sleep(100);
                if (ret != ReactorReturnCode.SUCCESS)
                {
                    return ReactorReturnCode.FAILURE;
                }

                if(m_ProviderSessionOptions.SendMarketDataItemUpdate && requestMsg.CheckStreaming())
                {
                    Thread.Sleep(4000);

                    ret = SendItemUpdate(chnl, msg);
                    if (ret != ReactorReturnCode.SUCCESS)
                    {
                        return ReactorReturnCode.FAILURE;
                    }
                }

                MarketPriceItemIndex++;
            }

            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode SendItemResponse(ReactorChannel chnl, Eta.Codec.Msg msg)
        {
            m_RefreshMsg.Clear();
            m_RefreshMsg.StreamId = msg.StreamId;
            m_RefreshMsg.DomainType = (int)Eta.Rdm.DomainType.MARKET_PRICE;
            m_RefreshMsg.MsgClass = MsgClasses.REFRESH;
            m_RefreshMsg.ApplyRefreshComplete();
            m_RefreshMsg.ApplyClearCache();
            m_RefreshMsg.ApplySolicited();

            // Itemname
            m_RefreshMsg.ApplyHasMsgKey();
            m_RefreshMsg.MsgKey.Name = msg.MsgKey.Name;
            m_RefreshMsg.MsgKey.ApplyHasName();
            m_RefreshMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
            m_RefreshMsg.MsgKey.ApplyHasNameType();

            // Service ID
            m_RefreshMsg.MsgKey.ApplyHasServiceId();
            m_RefreshMsg.MsgKey.ServiceId = ServiceId;

            // State
            m_RefreshMsg.State.StreamState(StreamStates.OPEN);
            m_RefreshMsg.State.DataState(DataStates.OK);
            m_RefreshMsg.State.Code(StateCodes.NONE);
            m_RefreshMsg.State.Text().Data("Item Refresh Completed");

            // QoS
            m_RefreshMsg.ApplyHasQos();
            m_RefreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            m_RefreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);

            m_RefreshMsg.ContainerType = DataTypes.FIELD_LIST;
            m_FieldList.Clear();

            MarketPriceItems[MarketPriceItemIndex].Clear();

            ITransportBuffer? msgBuf = chnl.GetBuffer(MSG_BUFFER_SIZE, false, out var _);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            if (EncodeMsg(msgBuf, m_EncodeIterator, chnl, (Eta.Codec.Msg)m_RefreshMsg) != CodecReturnCode.SUCCESS)
            {
                return ReactorReturnCode.FAILURE;
            }

            m_SubmitOptions.Clear();

            return chnl.Submit(msgBuf, m_SubmitOptions, out _);
        }

        internal ReactorReturnCode ProcessGenericMsg(ReactorChannel chnl, Eta.Codec.Msg msg, DecodeIterator m_DecodeIterator)
        {
            m_GenericMsg.Clear();
            m_GenericMsg.MsgClass = MsgClasses.GENERIC;
            m_GenericMsg.StreamId = msg.StreamId;
            m_GenericMsg.DomainType = msg.DomainType;
            m_GenericMsg.ContainerType = DataTypes.ELEMENT_LIST;
            m_GenericMsg.ApplyHasMsgKey();
            m_GenericMsg.MsgKey.ApplyHasName();
            m_GenericMsg.MsgKey.Name = msg.MsgKey.Name;

            ByteBuffer byteBuffer = new (msg.EncodedDataBody.Length);

            byteBuffer.Put(msg.EncodedDataBody.Data().Contents, msg.EncodedDataBody.Position, 
                msg.EncodedDataBody.Length); byteBuffer.Flip();

            Buffer dataBodyBuf = new();
            dataBodyBuf.Data(byteBuffer);

            m_GenericMsg.EncodedDataBody = dataBodyBuf;

            m_SubmitOptions.Clear();
            return chnl.Submit(m_GenericMsg, m_SubmitOptions, out _);
        }

        private ReactorReturnCode SendItemUpdate(ReactorChannel chnl, Eta.Codec.Msg msg)
        {
            m_UpdateMsg.Clear();
            m_UpdateMsg.StreamId = msg.StreamId;
            m_UpdateMsg.DomainType = (int)Eta.Rdm.DomainType.MARKET_PRICE;
            m_UpdateMsg.MsgClass = MsgClasses.UPDATE;
            m_UpdateMsg.ContainerType = DataTypes.FIELD_LIST;

            ITransportBuffer? msgBuf = chnl.GetBuffer(MSG_BUFFER_SIZE, false, out var _);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            if (EncodeMsg(msgBuf, m_EncodeIterator, chnl, (Eta.Codec.Msg)m_UpdateMsg) != CodecReturnCode.SUCCESS)
            {
                return ReactorReturnCode.FAILURE;
            }

            m_SubmitOptions.Clear();

            return chnl.Submit(msgBuf, m_SubmitOptions, out _);
        }

        public ReactorReturnCode ProcessBatchRequest(ReactorChannel chnl, Eta.Codec.Msg msg, DecodeIterator dIter, bool isPrivateStream)
        {
            throw new NotImplementedException();
        }

        public ReactorReturnCode SendItemRequestReject(ReactorChannel channel, int streamId, int domainType, ItemRejectReason reason, bool isPrivateStream)
        {
            ITransportBuffer? msgBuf = channel.GetBuffer(REJECT_MSG_SIZE, false, out _);
            if (msgBuf == null)
                return ReactorReturnCode.FAILURE;

            m_StatusMsg.Clear();
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.ApplyHasState();
            m_StatusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            m_StatusMsg.State.DataState(DataStates.SUSPECT);

            if (isPrivateStream)
                m_StatusMsg.ApplyPrivateStream();

            //encode request reject status
            switch (reason)
            {
                case ItemRejectReason.ITEM_COUNT_REACHED:
                    m_StatusMsg.State.Code(StateCodes.TOO_MANY_ITEMS);
                    m_StatusMsg.State.Text().Data($"Item request rejected for stream id {streamId}- item count reached for this channel");
                    break;
                case ItemRejectReason.INVALID_SERVICE_ID:
                    m_StatusMsg.State.Code(StateCodes.USAGE_ERROR);
                    m_StatusMsg.State.StreamState(StreamStates.CLOSED);
                    m_StatusMsg.State.Text().Data($"Item request rejected for stream id {streamId}- service id invalid");
                    break;
                case ItemRejectReason.QOS_NOT_SUPPORTED:
                    m_StatusMsg.State.Code(StateCodes.USAGE_ERROR);
                    m_StatusMsg.State.StreamState(StreamStates.CLOSED);
                    m_StatusMsg.State.Text().Data($"Item request rejected for stream id {streamId}- QoS not supported");
                    break;
                case ItemRejectReason.ITEM_ALREADY_OPENED:
                    m_StatusMsg.State.Code(StateCodes.ALREADY_OPEN);
                    m_StatusMsg.State.StreamState(StreamStates.CLOSED);
                    m_StatusMsg.State.Text().Data($"Item request rejected for stream id {streamId}- item already open with exact same key on another stream");
                    break;
                case ItemRejectReason.STREAM_ALREADY_IN_USE:
                    m_StatusMsg.State.Code(StateCodes.USAGE_ERROR);
                    m_StatusMsg.State.StreamState(StreamStates.CLOSED);
                    m_StatusMsg.State.Text().Data($"Item request rejected for stream id {streamId}- stream already in use with a different key");
                    break;
                case ItemRejectReason.KEY_ENC_ATTRIB_NOT_SUPPORTED:
                    m_StatusMsg.State.Code(StateCodes.USAGE_ERROR);
                    m_StatusMsg.State.StreamState(StreamStates.CLOSED);
                    m_StatusMsg.State.Text().Data($"Item request rejected for stream id {streamId}- this provider does not support key attribute information");
                    break;
                case ItemRejectReason.ITEM_NOT_SUPPORTED:
                    m_StatusMsg.State.Code(StateCodes.USAGE_ERROR);
                    m_StatusMsg.State.StreamState(StreamStates.CLOSED);
                    m_StatusMsg.State.Text().Data($"Item request rejected for stream id {streamId}- item not supported");
                    break;
                case ItemRejectReason.PRIVATE_STREAM_REDIRECT:
                    m_StatusMsg.ApplyPrivateStream();
                    m_StatusMsg.State.Code(StateCodes.NONE);
                    m_StatusMsg.State.StreamState(StreamStates.REDIRECTED);
                    m_StatusMsg.State.Text().Data($"Standard stream redirect to private for stream id {streamId} - this item must be requested via private stream");
                    break;
                case ItemRejectReason.PRIVATE_STREAM_MISMATCH:
                    m_StatusMsg.ApplyPrivateStream();
                    m_StatusMsg.State.Code(StateCodes.USAGE_ERROR);
                    m_StatusMsg.State.StreamState(StreamStates.CLOSED);
                    m_StatusMsg.State.Text().Data($"Rejected request for stream id {streamId} - reissue via batch request is not allowed");
                    break;
                default:
                    break;
            }

            m_EncodeIterator.Clear();
            m_StatusMsg.StreamId = streamId;
            m_StatusMsg.DomainType = domainType;
            CodecReturnCode ret = m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ReactorReturnCode.FAILURE;
            }

            ret = m_StatusMsg.Encode(m_EncodeIterator);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ReactorReturnCode.FAILURE;
            }

            m_SubmitOptions.Clear();

            return channel.Submit(msgBuf, m_SubmitOptions, out _);
        }

        public CodecReturnCode EncodeMsg(ITransportBuffer msgBuf,EncodeIterator encodeIterator, ReactorChannel chnl, Eta.Codec.Msg msg)
        {
            encodeIterator.Clear();
            encodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

            CodecReturnCode ret = msg.EncodeInit(encodeIterator, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            if (msg.MsgClass == MsgClasses.REFRESH)
            {
                ret = EncodeRefreshFields(encodeIterator);
                if (ret < CodecReturnCode.SUCCESS)
                    return ret;
            }
            else if (msg.MsgClass == MsgClasses.UPDATE)
            {
                ret = EncodeUpdateFields(encodeIterator);
                if (ret < CodecReturnCode.SUCCESS)
                    return ret;
            }

            return msg.EncodeComplete(encodeIterator, true);
        }

        private CodecReturnCode EncodeUpdateFields(EncodeIterator encodeIterator)
        {
            IDictionaryEntry dictionaryEntry;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

            MarketPriceItems[MarketPriceItemIndex].UpdateFields();

            m_FieldList.ApplyHasStandardData();
            ret = m_FieldList.EncodeInit(encodeIterator, null, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            // TRDPRC_1
            if (ViewFids.Count == 0 || ViewFids.Contains(MarketPriceItem.TRDPRC_1_FID))
            {
                m_FieldEntry.Clear();
                dictionaryEntry = m_DataDictionary!.Entry(MarketPriceItem.TRDPRC_1_FID);
                if (dictionaryEntry != null)
                {
                    m_FieldEntry.FieldId = MarketPriceItem.TRDPRC_1_FID;
                    m_FieldEntry.DataType = dictionaryEntry.GetRwfType();
                    m_Real.Clear();
                    m_Real.Value(MarketPriceItems[MarketPriceItemIndex].TRDPRC_1, RealHints.EXPONENT_2);
                    ret = m_FieldEntry.Encode(encodeIterator, m_Real);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            // BID
            if (ViewFids.Count == 0 || ViewFids.Contains(MarketPriceItem.BID_FID))
            {
                m_FieldEntry.Clear();
                dictionaryEntry = m_DataDictionary.Entry(MarketPriceItem.BID_FID);
                if (dictionaryEntry != null)
                {
                    m_FieldEntry.FieldId = MarketPriceItem.BID_FID;
                    m_FieldEntry.DataType = dictionaryEntry.GetRwfType();
                    m_Real.Clear();
                    m_Real.Value(MarketPriceItems[MarketPriceItemIndex].BID, RealHints.EXPONENT_2);
                    ret = m_FieldEntry.Encode(encodeIterator, m_Real);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            // ASK
            if (ViewFids.Count == 0 || ViewFids.Contains(MarketPriceItem.ASK_FID))
            {
                m_FieldEntry.Clear();
                dictionaryEntry = m_DataDictionary.Entry(MarketPriceItem.ASK_FID);
                if (dictionaryEntry != null)
                {
                    m_FieldEntry.FieldId = MarketPriceItem.ASK_FID;
                    m_FieldEntry.DataType = dictionaryEntry.GetRwfType();
                    m_Real.Clear();
                    m_Real.Value(MarketPriceItems[MarketPriceItemIndex].ASK, RealHints.EXPONENT_2);
                    ret = m_FieldEntry.Encode(encodeIterator, m_Real);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            ret = m_FieldList.EncodeComplete(encodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            return ret;
        }

        private CodecReturnCode EncodeRefreshFields(EncodeIterator encodeIterator)
        {
            IDictionaryEntry dictionaryEntry;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

            MarketPriceItems[MarketPriceItemIndex].InitFields();

            m_FieldList.ApplyHasStandardData();
            ret = m_FieldList.EncodeInit(encodeIterator, null, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            // RDNDISPLAY
            if (ViewFids.Count == 0 || ViewFids.Contains(MarketPriceItem.RDNDISPLAY_FID))
            {
                m_FieldEntry.Clear();
                dictionaryEntry = m_DataDictionary.Entry(MarketPriceItem.RDNDISPLAY_FID);
                if (dictionaryEntry != null)
                {
                    m_FieldEntry.FieldId = MarketPriceItem.RDNDISPLAY_FID;
                    m_FieldEntry.DataType = dictionaryEntry.GetRwfType();
                    m_UInt.Value(MarketPriceItems[MarketPriceItemIndex].RDNDISPLAY);
                    if ((ret = m_FieldEntry.Encode(encodeIterator, m_UInt)) < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            // RDN_EXCHID
            if (ViewFids.Count == 0 || ViewFids.Contains(MarketPriceItem.RDN_EXCHID_FID))
            {
                m_FieldEntry.Clear();
                dictionaryEntry = m_DataDictionary.Entry(MarketPriceItem.RDN_EXCHID_FID);
                if (dictionaryEntry != null)
                {
                    m_FieldEntry.FieldId = MarketPriceItem.RDN_EXCHID_FID;
                    m_FieldEntry.DataType = dictionaryEntry.GetRwfType();
                    Enum enumValue = new Enum();
                    enumValue.Value(MarketPriceItems[MarketPriceItemIndex].RDN_EXCHID);
                    if ((ret = m_FieldEntry.Encode(encodeIterator, enumValue)) < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            // DIVPAYDATE
            if (ViewFids.Count == 0 || ViewFids.Contains(MarketPriceItem.DIVPAYDATE_FID))
            {
                m_FieldEntry.Clear();
                dictionaryEntry = m_DataDictionary.Entry(MarketPriceItem.DIVPAYDATE_FID);
                if (dictionaryEntry != null)
                {
                    m_FieldEntry.FieldId = MarketPriceItem.DIVPAYDATE_FID;
                    m_FieldEntry.DataType = dictionaryEntry.GetRwfType();
                    if ((ret = m_FieldEntry.Encode(encodeIterator, MarketPriceItems[MarketPriceItemIndex].DIVPAYDATE)) < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            // TRDPRC_1
            if (ViewFids.Count == 0 || ViewFids.Contains(MarketPriceItem.TRDPRC_1_FID))
            {
                m_FieldEntry.Clear();
                dictionaryEntry = m_DataDictionary!.Entry(MarketPriceItem.TRDPRC_1_FID);
                if (dictionaryEntry != null)
                {
                    m_FieldEntry.FieldId = MarketPriceItem.TRDPRC_1_FID;
                    m_FieldEntry.DataType = dictionaryEntry.GetRwfType();
                    m_Real.Clear();
                    m_Real.Value(MarketPriceItems[MarketPriceItemIndex].TRDPRC_1, RealHints.EXPONENT_2);
                    ret = m_FieldEntry.Encode(encodeIterator, m_Real);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            // BID
            if (ViewFids.Count == 0 || ViewFids.Contains(MarketPriceItem.BID_FID))
            {
                m_FieldEntry.Clear();
                dictionaryEntry = m_DataDictionary.Entry(MarketPriceItem.BID_FID);
                if (dictionaryEntry != null)
                {
                    m_FieldEntry.FieldId = MarketPriceItem.BID_FID;
                    m_FieldEntry.DataType = dictionaryEntry.GetRwfType();
                    m_Real.Clear();
                    m_Real.Value(MarketPriceItems[MarketPriceItemIndex].BID, RealHints.EXPONENT_2);
                    ret = m_FieldEntry.Encode(encodeIterator, m_Real);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            // ASK
            if (ViewFids.Count == 0 || ViewFids.Contains(MarketPriceItem.ASK_FID))
            {
                m_FieldEntry.Clear();
                dictionaryEntry = m_DataDictionary.Entry(MarketPriceItem.ASK_FID);
                if (dictionaryEntry != null)
                {
                    m_FieldEntry.FieldId = MarketPriceItem.ASK_FID;
                    m_FieldEntry.DataType = dictionaryEntry.GetRwfType();
                    m_Real.Clear();
                    m_Real.Value(MarketPriceItems[MarketPriceItemIndex].ASK, RealHints.EXPONENT_2);
                    ret = m_FieldEntry.Encode(encodeIterator, m_Real);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            // ASK_TIME
            if (ViewFids.Count == 0 || ViewFids.Contains(MarketPriceItem.ASK_TIME_FID))
            {
                m_FieldEntry.Clear();
                dictionaryEntry = m_DataDictionary.Entry(MarketPriceItem.ASK_TIME_FID);
                if (dictionaryEntry != null)
                {
                    m_FieldEntry.FieldId = MarketPriceItem.ASK_TIME_FID;
                    m_FieldEntry.DataType = dictionaryEntry.GetRwfType();
                    ret = m_FieldEntry.Encode(encodeIterator, MarketPriceItems[MarketPriceItemIndex].ASK_TIME.Time());
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            ret = m_FieldList.EncodeComplete(encodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            return ret;
        }
    }
}
