/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using System.Diagnostics;
using LSEG.Eta.Rdm;

namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// Provides encoding of messages containing item data, 
    /// in the forms of refreshes, updates, posts, and generic messages.
    /// </summary>
    public class ItemEncoder
    {
        private EncodeIterator m_EncIter = new EncodeIterator();
        private IPostMsg m_PostMsg = (IPostMsg)new Msg();
        private IGenericMsg m_GenMsg = (IGenericMsg)new Msg();
        private MarketPriceEncoder m_MarketPriceEncoder;
        private XmlMsgData m_XmlMsgData;
        private Msg m_TmpMsg = new Msg();

        public ItemEncoder(XmlMsgData msgData)
        {
            m_XmlMsgData = msgData;
            m_MarketPriceEncoder = new MarketPriceEncoder(m_XmlMsgData);
        }

        /// <summary>
        /// Encodes a Post message for an item.
        /// </summary>
        /// <param name="channel">channel to encode post message for</param>
        /// <param name="itemInfo">market data item to encode post message for</param>
        /// <param name="msgBuf"><see cref="ITransportBuffer"/> to encode post message into</param>
        /// <param name="postUserInfo">the post user info</param>
        /// <param name="encodeStartTime">if &gt;0, this is a latency timestamp to be included in the post message</param>
        /// <returns>&lt;0 if encoding fails, 0 otherwise</returns>
        public CodecReturnCode EncodeItemPost(IChannel channel, ItemInfo itemInfo, ITransportBuffer msgBuf, PostUserInfo postUserInfo, long encodeStartTime)
        {
            m_EncIter.Clear();
            CodecReturnCode codecReturnCode = m_EncIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (codecReturnCode == CodecReturnCode.SUCCESS)
            {
                return EncodeItemPost(itemInfo, m_PostMsg, m_EncIter, postUserInfo, encodeStartTime);
            }  
            else
            {
                Console.WriteLine($"SetBufferAndRWFVersion() failed: {codecReturnCode.GetAsString()}");
                return codecReturnCode;
            }
        }

        /// <summary>
        /// Creates a Post message for an item.
        /// </summary>
        /// <param name = "channel" > channel to encode post message for</param>
        /// <param name="itemInfo">market data item to encode post message for</param>
        /// <param name="postMsg">PostMsg to create post message into</param>
        /// <param name="postBuffer">buffer for UpdateMsg in encoded data body</param>
        /// <param name="postUserInfo">the post user info</param>
        /// <param name="encodeStartTime">if &gt;0, this is a latency timestamp to be included in the post message</param>
        /// <returns>&lt;0 if encoding fails, 0 otherwise</returns>
        public CodecReturnCode CreateItemPostMsg(IChannel channel, ItemInfo itemInfo, IPostMsg postMsg, Codec.Buffer postBuffer, PostUserInfo postUserInfo, long encodeStartTime)
        {
            m_EncIter.Clear();
            CodecReturnCode codecReturnCode = m_EncIter.SetBufferAndRWFVersion(postBuffer, channel.MajorVersion, channel.MinorVersion);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"SetBufferAndRWFVersion() failed: {codecReturnCode.GetAsString()}.");
                return codecReturnCode;
            } 

            // Prepare Post message
            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = itemInfo.StreamId;
            postMsg.ApplyPostComplete();
            postMsg.PostUserInfo.UserAddr = postUserInfo.UserAddr;
            postMsg.PostUserInfo.UserId = postUserInfo.UserId;
            postMsg.DomainType = itemInfo.Attributes.DomainType;
            postMsg.ContainerType = DataTypes.MSG;

            // Prepare Update message
            m_TmpMsg.Clear();
            m_TmpMsg.MsgClass = MsgClasses.UPDATE;
            m_TmpMsg.ApplyHasPostUserInfo();
            m_TmpMsg.PostUserInfo.UserAddr = postUserInfo.UserAddr;
            m_TmpMsg.PostUserInfo.UserId = postUserInfo.UserId;
            m_TmpMsg.DomainType = itemInfo.Attributes.DomainType;

            if (itemInfo.Attributes.DomainType != (int)DomainType.MARKET_PRICE)
                return CodecReturnCode.FAILURE;
            m_TmpMsg.ContainerType = DataTypes.FIELD_LIST;
            if ((codecReturnCode = m_TmpMsg.EncodeInit(m_EncIter, 0)) < CodecReturnCode.SUCCESS)
                return codecReturnCode;
            if ((codecReturnCode = m_MarketPriceEncoder.EncodeDataBody(m_EncIter, m_MarketPriceEncoder.NextPostMsg((MarketPriceItem)itemInfo.ItemData!), MsgClasses.POST, encodeStartTime)) < CodecReturnCode.SUCCESS)
                return codecReturnCode;
            if ((codecReturnCode = m_TmpMsg.EncodeComplete(m_EncIter, true)) < CodecReturnCode.SUCCESS)
                return codecReturnCode;

            // set EncodedDataBody on PostMsg
            postMsg.EncodedDataBody = postBuffer;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes a Generic message for an item
        /// </summary>
        /// <param name="channel">channel to encode generic message for</param>
        /// <param name="itemInfo">market data item to encode generic message for</param>
        /// <param name="msgBuf"><see cref="ITransportBuffer"/> to encode generic message into</param>
        /// <param name="encodeStartTime">if &gt;0, this is a latency timestamp to be included in the generic message</param>
        /// <returns>&lt;0 if encoding fails, 0 otherwise</returns>
        public CodecReturnCode EncodeItemGenMsg(IChannel channel, ItemInfo itemInfo, ITransportBuffer msgBuf, long encodeStartTime)
        {
            m_EncIter.Clear();
            CodecReturnCode codecReturnCode = m_EncIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (codecReturnCode == CodecReturnCode.SUCCESS)
            {
                return EncodeItemGenMsg(itemInfo, m_GenMsg, m_EncIter, encodeStartTime);
            }
            else
            {
                Console.WriteLine($"SetBufferAndRWFVersion() failed: {codecReturnCode.GetAsString()}.");
                return codecReturnCode;
            }
        }

        /// <summary>
        /// Creates a Generic message for an item
        /// </summary>
        /// <param name="channel">channel to encode generic message for</param>
        /// <param name="itemInfo">market data item to encode generic message for</param>
        /// <param name="genericMsg">GenericMsg to create generic message into</param>
        /// <param name="genericBuffer"><see cref="ITransportBuffer"/> for UpdateMsg in encoded data body</param>
        /// <param name="encodeStartTime">if &gt;0, this is a latency timestamp to be included in the generic message</param>
        /// <returns>&lt;0 if encoding fails, 0 otherwise</returns>
        public CodecReturnCode CreateItemGenMsg(IChannel channel, ItemInfo itemInfo, IGenericMsg genericMsg, Codec.Buffer genericBuffer, long encodeStartTime)
        {
            m_EncIter.Clear();
            CodecReturnCode codecReturnCode = m_EncIter.SetBufferAndRWFVersion(genericBuffer, channel.MajorVersion, channel.MinorVersion);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"SetBufferAndRWFVersion() failed: {codecReturnCode.GetAsString()}.");
                return codecReturnCode;
            }

            genericMsg.MsgClass = MsgClasses.GENERIC;
            genericMsg.StreamId = itemInfo.StreamId;
            genericMsg.DomainType = itemInfo.Attributes.DomainType;
            if (itemInfo.Attributes.DomainType != (int)DomainType.MARKET_PRICE)
                return CodecReturnCode.FAILURE;
            genericMsg.ContainerType = DataTypes.FIELD_LIST;

            if ((codecReturnCode = m_MarketPriceEncoder.EncodeDataBody(m_EncIter, m_MarketPriceEncoder.NextGenMsg((MarketPriceItem)itemInfo.ItemData!), MsgClasses.GENERIC, encodeStartTime)) < CodecReturnCode.SUCCESS)
                return codecReturnCode;
            Debug.Assert(genericMsg.ContainerType == DataTypes.FIELD_LIST);

            genericMsg.EncodedDataBody = genericBuffer;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Estimates the length of a message class for an item
        /// </summary>
        /// <param name="itemInfo">item information to calculate message length</param>
        /// <param name="msgClass">the class of the message</param>
        /// <returns>estimated buffer size length</returns>
        public int EstimateItemMsgBufferLength(ItemInfo itemInfo, int msgClass)
        {
            int bufSize = 64;
            if (itemInfo.Attributes.DomainType == (int)DomainType.MARKET_PRICE)
            {
                int imsg = ((MarketPriceItem)itemInfo.ItemData!).IMsg;
                switch (msgClass)
                {
                    case MsgClasses.REFRESH:
                        bufSize += m_XmlMsgData.MarketPriceRefreshMsg!.EstimatedContentLength;
                        if (itemInfo.Attributes.MsgKey!.CheckHasName())
                            bufSize += itemInfo.Attributes.MsgKey.Name.Length;
                        if (itemInfo.Attributes.MsgKey.CheckHasAttrib())
                            bufSize += itemInfo.Attributes.MsgKey.EncodedAttrib.Length;
                        break;
                    case MsgClasses.UPDATE:
                        bufSize += m_XmlMsgData.MpUpdateMsgs![imsg].EstimatedContentLength;
                        break;
                    case MsgClasses.GENERIC:
                        bufSize += m_XmlMsgData.MpGenMsgs![imsg].EstimatedContentLength;
                        break;
                    case MsgClasses.POST:
                        bufSize += m_XmlMsgData.MpPostMsgs![imsg].EstimatedContentLength;
                        break;
                }
            }
            else
            {
                bufSize = 0;
            }
            return bufSize;
        }

        /// <summary>
        /// Encodes an Update message for an item
        /// </summary>
        /// <param name="channel">channel to encode update message for</param>
        /// <param name="itemInfo">market data item to encode update message for</param>
        /// <param name="msgBuf"><see cref="ITransportBuffer"/> to encode update message into</param>
        /// <param name="postUserInfo">the post user info</param>
        /// <param name="encodeStartTime">if &gt;0, this is a latency timestamp to be included in the update message</param>
        /// <param name="error">detailed error information in case of encoding failures</param>
        /// <returns>&lt;0 if encoding fails, 0 otherwise</returns>
        public CodecReturnCode EncodeUpdate(IChannel channel, ItemInfo itemInfo, ITransportBuffer msgBuf, PostUserInfo? postUserInfo, long encodeStartTime, out Error? error)
        {
            m_EncIter.Clear();
            CodecReturnCode codecReturnCode = m_EncIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (codecReturnCode == CodecReturnCode.SUCCESS)
            {
                return EncodeUpdate(itemInfo, postUserInfo, encodeStartTime, out error);
            } 
            else
            {
                error = new Error()
                {
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {codecReturnCode.GetAsString()}"
                };
                return codecReturnCode;
            }                         
        }

        /// <summary>
        /// Encodes a Update message for an item
        /// </summary>
        /// <param name="channel">channel to encode update message for</param>
        /// <param name="itemInfo">market data item to encode update message for</param>
        /// <param name="msgBuf"><see cref="Buffer"/> to encode update message into</param>
        /// <param name="postUserInfo">the post user info</param>
        /// <param name="encodeStartTime">if &gt;0, this is a latency timestamp to be included in the update message</param>
        /// <param name="error">detailed error information in case of encoding failures</param>
        /// <returns>&lt;0 if encoding fails, 0 otherwise</returns>
        public CodecReturnCode EncodeUpdate(IChannel channel, ItemInfo itemInfo, Codec.Buffer msgBuf, PostUserInfo? postUserInfo, long encodeStartTime, out Error? error)
        {
            m_EncIter.Clear();
            CodecReturnCode codecReturnCode = m_EncIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (codecReturnCode == CodecReturnCode.SUCCESS)
            {
                return EncodeUpdate(itemInfo, postUserInfo, encodeStartTime, out error);
            } 
            else
            {
                error = new Error()
                {
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {codecReturnCode.GetAsString()}"
                };
                return codecReturnCode;
            }         
        }

        /// <summary>
        /// Encodes a Refresh message for an item
        /// </summary>
        /// <param name="channel">channel to encode refresh message for</param>
        /// <param name="itemInfo">market data item to encode refresh message for</param>
        /// <param name="msgBuf"><see cref="ITransportBuffer"/> to encode refresh message into</param>
        /// <param name="postUserInfo">the post user info</param>
        /// <param name="encodeStartTime">if &gt;0, this is a latency timestamp to be included in the refresh message</param>
        /// <param name="error">detailed error information in case of encoding failures</param>
        /// <returns>&lt;0 if encoding fails, 0 otherwise</returns>
        public CodecReturnCode EncodeRefresh(IChannel channel, ItemInfo itemInfo, ITransportBuffer msgBuf, PostUserInfo? postUserInfo, long encodeStartTime, out Error? error)
        {
            m_EncIter.Clear();
            CodecReturnCode codecReturnCode = m_EncIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (codecReturnCode == CodecReturnCode.SUCCESS)
            {
                return EncodeRefresh(itemInfo, postUserInfo, encodeStartTime, out error);
            } 
            else
            {
                error = new Error()
                {
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {codecReturnCode.GetAsString()}"
                };
                return codecReturnCode;
            }           
        }

        /// <summary>
        /// Encodes a Refresh message for an item
        /// </summary>
        /// <param name="channel">channel to encode refresh message for</param>
        /// <param name="itemInfo">market data item to encode refresh message for</param>
        /// <param name="msgBuf"><see cref="Codec.Buffer"/> to encode refresh message into</param>
        /// <param name="postUserInfo">the post user info</param>
        /// <param name="encodeStartTime">if &gt;0, this is a latency timestamp to be included in the refresh message</param>
        /// <param name="error">detailed error information in case of encoding failures</param>
        /// <returns>&lt;0 if encoding fails, 0 otherwise</returns>
        public CodecReturnCode EncodeRefresh(IChannel channel, ItemInfo itemInfo, Codec.Buffer msgBuf, PostUserInfo? postUserInfo, long encodeStartTime, out Error? error)
        {
            m_EncIter.Clear();
            CodecReturnCode codecReturnCode = m_EncIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (codecReturnCode == CodecReturnCode.SUCCESS)
            {
                return EncodeRefresh(itemInfo, postUserInfo, encodeStartTime, out error);
            }
            else
            {
                error = new Error()
                {
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {codecReturnCode.GetAsString()}"
                };
                return codecReturnCode;
            }          
        }

        private CodecReturnCode EncodeItemGenMsg(ItemInfo itemInfo, IGenericMsg genMsg, EncodeIterator encIter, long encodeStartTime)
        {
            genMsg.MsgClass = MsgClasses.GENERIC;
            genMsg.StreamId = itemInfo.StreamId;
            genMsg.DomainType = itemInfo.Attributes.DomainType;
            if (itemInfo.Attributes.DomainType != (int)DomainType.MARKET_PRICE)
                return CodecReturnCode.FAILURE;
            genMsg.ContainerType = DataTypes.FIELD_LIST;
            CodecReturnCode codecReturnCode = genMsg.EncodeInit(encIter, 0);
            if (codecReturnCode < CodecReturnCode.SUCCESS)
                return codecReturnCode;
            if ((codecReturnCode = m_MarketPriceEncoder.EncodeDataBody(encIter, m_MarketPriceEncoder.NextGenMsg((MarketPriceItem)itemInfo.ItemData!), MsgClasses.GENERIC, encodeStartTime)) < CodecReturnCode.SUCCESS)
                return codecReturnCode;
            Debug.Assert(genMsg.ContainerType == DataTypes.FIELD_LIST); 
            return genMsg.EncodeComplete(encIter, true);
        }

        private CodecReturnCode EncodeItemPost(ItemInfo itemInfo, IPostMsg postMsg, EncodeIterator encIter, PostUserInfo postUserInfo, long encodeStartTime)
        {
            postMsg.MsgClass = MsgClasses.POST;
            postMsg.StreamId = itemInfo.StreamId;
            postMsg.ApplyPostComplete();
            postMsg.PostUserInfo.UserAddr = postUserInfo.UserAddr;
            postMsg.PostUserInfo.UserId = postUserInfo.UserId;
            postMsg.DomainType = itemInfo.Attributes.DomainType;
            postMsg.ContainerType = DataTypes.MSG;
            m_TmpMsg.Clear();
            m_TmpMsg.MsgClass = MsgClasses.UPDATE;
            m_TmpMsg.ApplyHasPostUserInfo();
            m_TmpMsg.PostUserInfo.UserAddr = postUserInfo.UserAddr;
            m_TmpMsg.PostUserInfo.UserId = postUserInfo.UserId;
            m_TmpMsg.DomainType = itemInfo.Attributes.DomainType;
            CodecReturnCode codecReturnCode = postMsg.EncodeInit(encIter, 0);
            if (codecReturnCode < CodecReturnCode.SUCCESS)
                return codecReturnCode;
            if (itemInfo.Attributes.DomainType != (int)DomainType.MARKET_PRICE)
                return CodecReturnCode.FAILURE;
            m_TmpMsg.ContainerType = DataTypes.FIELD_LIST;
            if ((codecReturnCode = m_TmpMsg.EncodeInit(encIter, 0)) < CodecReturnCode.SUCCESS)
                return codecReturnCode;
            if ((codecReturnCode = m_MarketPriceEncoder.EncodeDataBody(encIter, m_MarketPriceEncoder.NextPostMsg((MarketPriceItem)itemInfo.ItemData!), MsgClasses.POST, encodeStartTime)) < CodecReturnCode.SUCCESS)
                return codecReturnCode;
            if ((codecReturnCode = m_TmpMsg.EncodeComplete(encIter, true)) < CodecReturnCode.SUCCESS)
                return codecReturnCode;
            return postMsg.EncodeComplete(encIter, true);
        }

        private CodecReturnCode EncodeUpdate(ItemInfo itemInfo, PostUserInfo? postUserInfo, long encodeStartTime, out Error? error)
        {
            m_TmpMsg.Clear();
            m_TmpMsg.MsgClass = MsgClasses.UPDATE;
            m_TmpMsg.DomainType = itemInfo.Attributes.DomainType;
            m_TmpMsg.StreamId = itemInfo.StreamId;
            if (postUserInfo != null)
            {
                m_TmpMsg.ApplyHasPostUserInfo();
                m_TmpMsg.PostUserInfo.UserAddr = postUserInfo.UserAddr;
                m_TmpMsg.PostUserInfo.UserId = postUserInfo.UserId;
            }
            switch (itemInfo.Attributes.DomainType)
            {
                case (int)DomainType.MARKET_PRICE:
                    m_TmpMsg.ContainerType = DataTypes.FIELD_LIST;
                    CodecReturnCode codecReturnCode = m_TmpMsg.EncodeInit(m_EncIter, 0);
                    if (codecReturnCode < CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"UpdateMsg.EncodeInit() failed with return code: {codecReturnCode.GetAsString()}"
                        };
                        return codecReturnCode;
                    }
                    MarketPriceItem itemData = (MarketPriceItem)itemInfo.ItemData!;
                    int index = itemData.IMsg;
                    MarketPriceMsg[] mpUpdateMsgs = m_XmlMsgData.MpUpdateMsgs!;
                    MarketPriceMsg mpMsg = mpUpdateMsgs[index++];
                    if (index == m_XmlMsgData.UpdateCount)
                        index = 0;
                    itemData.IMsg = index;
                    codecReturnCode = m_MarketPriceEncoder.EncodeDataBody(m_EncIter, mpMsg, MsgClasses.UPDATE, encodeStartTime);
                    if (codecReturnCode < CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"MarketPriceEncoder.EncodeDataBody() failed with return code: {codecReturnCode.GetAsString()}"
                        };
                        return codecReturnCode;
                    }
                    codecReturnCode = m_TmpMsg.EncodeComplete(m_EncIter, true);
                    if (codecReturnCode < CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"UpdateMsg.EncodeComplete() failed with return code: {codecReturnCode.GetAsString()}"
                        };
                        return codecReturnCode;
                    }
                    error = null;
                    return codecReturnCode;
                default:
                    error = new Error()
                    {
                        Text = $"Unsupported DomainType: {itemInfo.Attributes.DomainType.ToString()}"
                    };
                    return CodecReturnCode.FAILURE;
            }
        }

        private CodecReturnCode EncodeRefresh(ItemInfo itemInfo, PostUserInfo? postUserInfo, long encodeStartTime, out Error? error)
        {
            m_TmpMsg.Clear();
            m_TmpMsg.MsgClass = MsgClasses.REFRESH;
            m_TmpMsg.DomainType = itemInfo.Attributes.DomainType;
            m_TmpMsg.Flags = RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.HAS_QOS;
            
            if ((itemInfo.ItemFlags & (int)ItemFlags.IS_SOLICITED) != 0)
            {
                m_TmpMsg.ApplySolicited();
                m_TmpMsg.ApplyClearCache();
            }
            itemInfo.Attributes.MsgKey!.Copy(m_TmpMsg.MsgKey);
            m_TmpMsg.StreamId = itemInfo.StreamId;

            m_TmpMsg.State.StreamState((itemInfo.ItemFlags & (int)ItemFlags.IS_STREAMING_REQ) != 0 ? StreamStates.OPEN : StreamStates.NON_STREAMING);
            m_TmpMsg.State.DataState(DataStates.OK);
            m_TmpMsg.State.Code(StateCodes.NONE);
            m_TmpMsg.State.Text().Data("Item Refresh Completed");

            m_TmpMsg.Qos.IsDynamic = false;
            m_TmpMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            m_TmpMsg.Qos.Timeliness(QosTimeliness.REALTIME);

            if (postUserInfo != null)
            {
                m_TmpMsg.ApplyHasPostUserInfo();
                m_TmpMsg.PostUserInfo.UserAddr = postUserInfo.UserAddr;
                m_TmpMsg.PostUserInfo.UserId = postUserInfo.UserId;
            }
            switch (itemInfo.Attributes.DomainType)
            {
                case (int)DomainType.MARKET_PRICE:
                    m_TmpMsg.ContainerType = DataTypes.FIELD_LIST;
                    CodecReturnCode codecReturnCode = m_TmpMsg.EncodeInit(m_EncIter, 0);
                    if (codecReturnCode < CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"RefreshMsg.EncodeInit() failed with return code: {codecReturnCode.GetAsString()}"
                        };
                        return codecReturnCode;
                    }
                    codecReturnCode = m_MarketPriceEncoder.EncodeDataBody(m_EncIter, m_XmlMsgData.MarketPriceRefreshMsg!, MsgClasses.REFRESH, encodeStartTime);
                    if (codecReturnCode < CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"MarketPriceEncoder.EncodeDataBody() failed with return code: {codecReturnCode.GetAsString()}"
                        };
                        return codecReturnCode;
                    }
                    codecReturnCode = m_TmpMsg.EncodeComplete(m_EncIter, true);
                    if (codecReturnCode < CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"RefreshMsg.EncodeComplete() failed with return code: {codecReturnCode.GetAsString()}"
                        };
                        return codecReturnCode;
                    }
                    error = null;
                    return codecReturnCode;
                default:
                    error = new Error()
                    {
                        Text = $"Unsupported DomainType: {itemInfo.Attributes.DomainType.ToString()}"
                    };
                    return CodecReturnCode.FAILURE;
            }
        }
    }
}
