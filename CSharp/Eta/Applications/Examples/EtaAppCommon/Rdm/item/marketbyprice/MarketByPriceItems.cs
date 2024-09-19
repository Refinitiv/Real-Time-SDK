/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using System.Collections.Generic;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// This handles storage of all market by price items.
    /// <remarks>
    /// Provides methods generation of market by price data, for managing the list, and a method for encoding a
    /// market by price message and payload.
    /// </remarks>
    /// </summary>
    public class MarketByPriceItems
    {
        private const int MAX_MARKET_PRICE_ITEM_LIST_SIZE = 100;

        // item information list
        private List<MarketByPriceItem> m_MarketByPriceList = new List<MarketByPriceItem>(MAX_MARKET_PRICE_ITEM_LIST_SIZE);

        private static Enum USD_ENUM;
        private static Enum BBO_ENUM;

        protected const int MAX_ORDERS = 3; //Number of order in a single message

        protected MarketByPriceRefresh m_MarketByPriceRefresh = new MarketByPriceRefresh();
        protected MarketByPriceUpdate m_MarketByPriceUpdate = new MarketByPriceUpdate();

        protected EncodeIterator m_EncodeIter = new EncodeIterator();

        static MarketByPriceItems()
        {
            USD_ENUM = new Enum();
            USD_ENUM.Value(840);
            BBO_ENUM = new Enum();
            BBO_ENUM.Value(20);
        }

        /// <summary>
        /// Instantiates a new market by price items.
        /// </summary>
        public MarketByPriceItems()
        {
            for (int i = 0; i < MAX_MARKET_PRICE_ITEM_LIST_SIZE; i++)
            {
                m_MarketByPriceList.Add(new MarketByPriceItem());
            }
        }

        /// <summary>
        /// Initializes market price item list.
        /// </summary>
        public void Init()
        {
            // clear item information list 
            foreach (MarketByPriceItem mbpItem in m_MarketByPriceList)
            {
                mbpItem.Clear();
            }
        }

        /// <summary>
        /// Updates any item that's currently in use.
        /// </summary>
        public void Update()
        {
            foreach (MarketByPriceItem mbpItem in m_MarketByPriceList)
            {
                if (mbpItem.IsInUse)
                    mbpItem.UpdateFields();
            }
        }

        /// <summary>
        /// Gets storage for a market by price item from the list.
        /// </summary>
        /// <param name="itemName">the item name</param>
        /// <returns>the market by price item</returns>
        public MarketByPriceItem? Get(string itemName)
        {
            // first try to find one with same name and reuse
            foreach (MarketByPriceItem mbpItem in m_MarketByPriceList)
            {
                if (mbpItem.IsInUse && mbpItem.ItemName != null && mbpItem.ItemName.Equals(itemName))
                {
                    return mbpItem;
                }
            }

            // next get a new one
            foreach (MarketByPriceItem mbpItem in m_MarketByPriceList)
            {
                if (!mbpItem.IsInUse)
                {
                    mbpItem.InitFields();
                    mbpItem.ItemName = itemName;
                    return mbpItem;
                }
            }

            return null;
        }

        /// <summary>
        /// Clears the item information.
        /// </summary>
        public void Clear()
        {
            foreach (MarketByPriceItem mbpItem in m_MarketByPriceList)
            {
                mbpItem.Clear();
            }
        }

        /// <summary>
        /// Encodes the market by price refresh
        /// </summary>
        /// <param name="channel">The channel to send a market by price refresh to</param>
        /// <param name="itemInfo">The item information</param>
        /// <param name="msgBuf">The message buffer to encode the market by price refresh into</param>
        /// <param name="isSolicited">The refresh is solicited if set</param>
        /// <param name="streamId">The stream id of the market by price refresh</param>
        /// <param name="isStreaming">Flag for streaming or snapshot</param>
        /// <param name="isPrivateStream">Flag for private stream</param>
        /// <param name="serviceId">The service id of the market by price refresh</param>
        /// <param name="dictionary">The dictionary used for encoding</param>
        /// <param name="multiPartNo">the multi part no</param>
        /// <param name="error">the error in an event of failure</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode EncodeRefresh(IChannel channel, ItemInfo itemInfo, ITransportBuffer msgBuf, bool isSolicited, int streamId, bool isStreaming,
            bool isPrivateStream, int serviceId, DataDictionary dictionary, int multiPartNo, out Error? error)
        {
            error = null;
            m_MarketByPriceRefresh.Clear();
            m_MarketByPriceRefresh.Dictionary = dictionary;
            m_MarketByPriceRefresh.ItemName.Data(itemInfo.ItemName.Data(), itemInfo.ItemName.Position, itemInfo.ItemName.Length);
            m_MarketByPriceRefresh.StreamId = streamId;
            m_MarketByPriceRefresh.HasServiceId = true;
            m_MarketByPriceRefresh.ServiceId = serviceId;
            if (isSolicited)
            {
                m_MarketByPriceRefresh.Solicited = true;

                // set clear cache on first part of all solicted refreshes.
                if (multiPartNo == 0)
                {
                    m_MarketByPriceRefresh.ClearCache = true;
                }
            }

            if (isPrivateStream)
                m_MarketByPriceRefresh.PrivateStream = true;


            m_MarketByPriceRefresh.Dictionary = dictionary;

            // QoS 
            m_MarketByPriceRefresh.Qos.IsDynamic = false;
            m_MarketByPriceRefresh.Qos.Timeliness(QosTimeliness.REALTIME);
            m_MarketByPriceRefresh.Qos.Rate(QosRates.TICK_BY_TICK);
            m_MarketByPriceRefresh.HasQos = true;

            // State 
            m_MarketByPriceRefresh.State.StreamState(isStreaming ? StreamStates.OPEN : StreamStates.NON_STREAMING);
            m_MarketByPriceRefresh.State.DataState(DataStates.OK);
            m_MarketByPriceRefresh.State.Code(StateCodes.NONE);

            // multi-part refresh complete when multiPartNo hits max
            if (multiPartNo == MAX_ORDERS - 1)
            {
                m_MarketByPriceRefresh.RefreshComplete = true;
                m_MarketByPriceRefresh.State.Text().Data("Item Refresh Completed");
            }
            else
            {
                m_MarketByPriceRefresh.State.Text().Data("Item Refresh In Progress");
            }
            m_MarketByPriceRefresh.PartNo = multiPartNo;
            m_MarketByPriceRefresh.MbpInfo = ((MarketByPriceItem)itemInfo.ItemData!);
            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text= $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }
            ret = m_MarketByPriceRefresh.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"MarketByPriceRefresh.Encode() failed"
                };
            }

            return ret;
        }

        /// <summary>
        /// Encodes the market by price update.
        /// </summary>
        /// <param name="channel">The channel to send a market by price update to</param>
        /// <param name="itemInfo">The item information</param>
        /// <param name="msgBuf">The message buffer to encode the market by price update into</param>
        /// <param name="isSolicited">The update is solicited if set</param>
        /// <param name="streamId">The stream id of the market by price update</param>
        /// <param name="isStreamingRequest">Flag for streaming or snapshot</param>
        /// <param name="isPrivateStreamRequest">Flag for private stream request</param>
        /// <param name="serviceId">The service id of the market by price update</param>
        /// <param name="dictionary">The dictionary used for encoding</param>
        /// <param name="error">error in case of encoding failure</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode EncodeUpdate(IChannel channel, ItemInfo itemInfo, ITransportBuffer msgBuf, bool isSolicited, int streamId,
            bool isStreamingRequest, bool isPrivateStreamRequest, int serviceId, DataDictionary dictionary, out Error? error)
        {
            error = null;
            m_MarketByPriceUpdate.Clear();
            m_MarketByPriceUpdate.RefreshComplete = true;
            if (isPrivateStreamRequest)
                m_MarketByPriceUpdate.PrivateStream = true;

            m_MarketByPriceUpdate.Dictionary = dictionary;
            m_MarketByPriceUpdate.ItemName.Data(itemInfo.ItemName.Data(), itemInfo.ItemName.Position, itemInfo.ItemName.Length);
            m_MarketByPriceUpdate.StreamId = streamId;
            m_MarketByPriceUpdate.Dictionary = dictionary;
            m_MarketByPriceUpdate.MbpInfo = ((MarketByPriceItem)itemInfo.ItemData!);
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
            ret = m_MarketByPriceUpdate.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"MarketByPriceUpdate.Encode() failed"
                };
            }

            return ret;
        }

    }
}
