/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using System;
using System.Collections.Generic;
using LSEG.Eta.Codec;
using Buffer = LSEG.Eta.Codec.Buffer;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;

namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// Represents one channel's session. Stores information about items requested.
    /// </summary>
    public class ProviderSession
    {
        // client sessions over this limit gets rejected with NAK mount
        public const int NUM_CLIENT_SESSIONS = 5;

        /// <summary>
        /// List of items to send refreshes for
        /// </summary>
        public ItemWatchlist RefreshItemList { get; set; }
        /// <summary>
        /// List of items to send updates for
        /// </summary>
        public ItemWatchlist UpdateItemList { get; set; }
        /// <summary>
        /// List of items to send generic messages for
        /// </summary>
        public ItemWatchlist GenMsgItemList { get; set; }
        /// <summary>
        /// Open items indexed by attributes
        /// </summary>
        public Dictionary<ItemAttributes, ItemInfo> ItemAttributesTable { get; set; }
        /// <summary>
        /// Open items indexed by stream id
        /// </summary>
        public Dictionary<int, ItemInfo> ItemStreamIdTable { get; set; }
        /// <summary>
        /// Count of items currently open
        /// </summary>
        public int OpenItemCount { get; set; }
        /// <summary>
        /// Client channel information
        /// </summary>
        public ClientChannelInfo? ClientChannelInfo { get; set; }
        /// <summary>
        /// Current buffer in use by this channel
        /// </summary>
        public ITransportBuffer? WritingBuffer { get; set; }
        /// <summary>
        /// Total number of buffers currently packed in writingBuffer
        /// </summary>
        public int PackedBufferCount { get; set; }
        /// <summary>
        /// Time at which this channel was fully setup
        /// </summary>
        public long TimeActivated { get; set; }
        /// <summary>
        /// Provider thread of this session
        /// </summary>
        public ProviderThread? ProviderThread { get; set; }
        /// <summary>
        /// This is used as temporary buffer for encoding RWF message for message packing
        /// </summary>
        public Buffer? TempBufferForPacking { get; set; }
        /// <summary>
        /// Keep track of the remaining packed buffer
        /// </summary>
        public int RemaingPackedBufferLength { get; set; }

        private ItemEncoder m_ItemEncoder;                                               // item encoder
        private XmlMsgData m_XmlMsgData;                                                 // Msgs from XML
        private int m_UnexpectedCloseCount;                                              // Count of unexpected close messages received

        public ProviderSession(XmlMsgData xmlMsgData, ItemEncoder itemEncoder)
        {
            RefreshItemList = new ItemWatchlist(100000);
            UpdateItemList = new ItemWatchlist(100000);
            GenMsgItemList = new ItemWatchlist(100000);
            ItemAttributesTable = new Dictionary<ItemAttributes, ItemInfo>(100000);
            ItemStreamIdTable = new Dictionary<int, ItemInfo>(100000);
            RemaingPackedBufferLength = 0;
            m_XmlMsgData = xmlMsgData;
            m_ItemEncoder = itemEncoder;
        }

        /// <summary>
        ///Initializes for one client channel
        /// </summary>
        /// <param name="clientChannelInfo">the client channel info</param>
        public void Init(ClientChannelInfo clientChannelInfo)
        {
            RefreshItemList.Init();
            UpdateItemList.Init();
            GenMsgItemList.Init();
            OpenItemCount = 0;
            PackedBufferCount = 0;
            TimeActivated = 0;
            ClientChannelInfo = clientChannelInfo;
            m_UnexpectedCloseCount = 0;
        }

        /// <summary>
        /// Clears provider session object
        /// </summary>
        public void Cleanup()
        {
            RefreshItemList.Clear();
            UpdateItemList.Clear();
            GenMsgItemList.Clear();
            OpenItemCount = 0;
            PackedBufferCount = 0;
            TimeActivated = 0;
            ClientChannelInfo = null;
            m_UnexpectedCloseCount = 0;
            TempBufferForPacking = null;
        }

        /// <summary>
        /// Creates item information to be published by provider
        /// </summary>
        /// <param name="attributes">item attributes</param>
        /// <param name="streamId">stream id</param>
        /// <returns>Created item information</returns>
        public ItemInfo? CreateItemInfo(ItemAttributes attributes, int streamId, out Error? error)
        {
            ItemInfo itemInfo = new ItemInfo();
            itemInfo.Attributes = attributes;
            itemInfo.StreamId = streamId;

            switch (attributes.DomainType)
            {
                case (int)DomainType.MARKET_PRICE:
                    if (m_XmlMsgData.UpdateCount == 0)
                    {
                        Console.Error.WriteLine("CreateItemInfo: No MarketPrice data present in message data file.");
                        error = new Error()
                        {
                            Text = "CreateItemInfo: No MarketPrice data present in message data file."
                        };
                        return null;
                    }
                    itemInfo.ItemData = new MarketPriceItem();
                    break;
                default:
                    error = new Error()
                    {
                        Text = $"Unsupported domain : {attributes.DomainType}"
                    };
                    Console.Error.WriteLine($"Unsupported domain : {attributes.DomainType}");
                    return null;
            }

            // Add item to watchlist
            RefreshItemList.Add(itemInfo);

            OpenItemCount++;

            error = null;
            return itemInfo;
        }

        /// <summary>
        /// Clears item information
        /// </summary>
        /// <param name="itemInfo">the item info</param>
        public void FreeItemInfo(ItemInfo itemInfo)
        {
            if (itemInfo == null)
                return;

            switch (itemInfo.Attributes.DomainType)
            {
                case (int)DomainType.MARKET_PRICE:
                    MarketPriceItem mpItem = (MarketPriceItem)itemInfo.ItemData!;
                    mpItem.IMsg = 0;
                    break;
                default:
                    break;
            }
            ItemAttributesTable.Remove(itemInfo.Attributes);
            ItemStreamIdTable.Remove(itemInfo.StreamId);
            if (itemInfo.Attributes.MsgKey!.CheckHasName())
            {
                itemInfo.Attributes.MsgKey.Name.Clear();
            }
            if (itemInfo.Attributes.MsgKey.CheckHasAttrib())
            {
                itemInfo.Attributes.MsgKey.EncodedAttrib.Clear();
            }
            itemInfo.Attributes.MsgKey.Clear();
            itemInfo.Clear();
            --OpenItemCount;
        }

        /// <summary>
        /// Prints estimated size of messages being published
        /// </summary>
        /// <param name="error"> Error information populated 
        /// when there is a failure to obtain buffers for measuring size</param>
        /// <returns><see cref="PerfToolsReturnCode.FAILURE"/> in case of failure, 
        /// <see cref="PerfToolsReturnCode.SUCCESS"/> otherwise</returns>
        public PerfToolsReturnCode PrintEstimatedMsgSizes(out Error? error)
        {
            MsgKey msgKey = new MsgKey();
            msgKey.Flags = MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_NAME;
            msgKey.NameType = InstrumentNameTypes.RIC;
            msgKey.Name.Data("RDT0");
            msgKey.ServiceId = 0;

            if (m_XmlMsgData.UpdateCount > 0)
            {
                MarketPriceItem mpItem = new MarketPriceItem();
                ItemInfo itemInfo = new ItemInfo();
                itemInfo.Attributes.MsgKey = msgKey;
                itemInfo.Attributes.DomainType = (int)DomainType.MARKET_PRICE;
                itemInfo.ItemData = mpItem;

                int bufLen = m_ItemEncoder.EstimateItemMsgBufferLength(itemInfo, MsgClasses.REFRESH);
                ITransportBuffer testBuffer = GetTempBuffer(bufLen, out error);
                if (testBuffer == null)
                    return PerfToolsReturnCode.FAILURE;
                CodecReturnCode ret = m_ItemEncoder.EncodeRefresh(ClientChannelInfo!.Channel!, itemInfo, testBuffer, null, 0, out error);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return PerfToolsReturnCode.FAILURE;
                }
                Console.WriteLine("Approximate message size:");
                Console.Write("  MarketPrice RefreshMsg (without name): \n");
                Console.Write($"         estimated length: {bufLen} bytes\n");
                Console.Write($"    approx encoded length: {testBuffer.Length()} bytes\n");
                ReleaseTempBuffer(testBuffer, out error);

                // Update msgs
                for (int i = 0; i < m_XmlMsgData.UpdateCount; ++i)
                {
                    bufLen = m_ItemEncoder.EstimateItemMsgBufferLength(itemInfo, MsgClasses.UPDATE);
                    testBuffer = GetTempBuffer(bufLen, out error);
                    if (testBuffer == null)
                        return PerfToolsReturnCode.FAILURE;
                    ret = m_ItemEncoder.EncodeUpdate(ClientChannelInfo.Channel!, itemInfo, testBuffer, null, 0, out error);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        return PerfToolsReturnCode.FAILURE;
                    }
                    Console.Write($"  MarketPrice UpdateMsg {i + 1}: \n");
                    Console.Write($"         estimated length: {bufLen} bytes\n");
                    Console.Write($"    approx encoded length: {testBuffer.Length()} bytes\n");
                    ReleaseTempBuffer(testBuffer, out error);
                    if (error != null)
                    {
                        Console.WriteLine($"Error releasing temp buffer: {error.Text}");
                        return PerfToolsReturnCode.FAILURE;
                    }
                }
            }

            Console.WriteLine();

            error = null;
            return PerfToolsReturnCode.SUCCESS;
        }

        /// <summary>
        /// Handles item stream close
        /// </summary>
        /// <param name="streamId">the stream id</param>
        public void CloseItemStream(int streamId)
        {
            if (ItemStreamIdTable.TryGetValue(streamId, out var itemInfo))
            {
                FreeItemInfo(itemInfo);
            }
            else
            {
                // If we are sending an update for an item to the platform while it
                // is sending us a close for that same item, it may respond to our
                // update with another close. If so, this is okay, so don't close
                // the channel because of it.
                if (m_UnexpectedCloseCount == 0)
                {
                    Console.Write($"Received unexpected close on stream {streamId} (this may just be an extra close from the platform). \n");
                }
                m_UnexpectedCloseCount++;
            }
        }

        private ITransportBuffer GetTempBuffer(int length, out Error error)
        {
            return ClientChannelInfo!.Channel!.GetBuffer(length, false, out error);
        }

        private void ReleaseTempBuffer(ITransportBuffer msgBuf, out Error error)
        {
            ClientChannelInfo!.Channel!.ReleaseBuffer(msgBuf, out error);
        }
    }
}
