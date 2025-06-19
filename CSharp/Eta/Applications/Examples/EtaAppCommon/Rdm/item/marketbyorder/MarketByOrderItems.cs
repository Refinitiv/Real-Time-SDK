/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using System.Collections.Generic;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// This handles storage of all market by order items.
    /// <remark>
    /// Provides methods generation of market by order data, for managing the list,
    /// and a method for encoding a market by order message and payload.
    /// </remark>
    /// </summary>
    public class MarketByOrderItems
    {
        private const int MAX_MARKET_PRICE_ITEM_LIST_SIZE = 100;

        // item information list
        public List<MarketByOrderItem> MarketByOrderList { get; private set; } = new List<MarketByOrderItem>();
        protected MarketByOrderRefresh m_MarketByOrderRefresh = new MarketByOrderRefresh();
        protected MarketByOrderUpdate m_MarketByOrderUpdate = new MarketByOrderUpdate();
        protected EncodeIterator m_EncodeIter = new EncodeIterator();

        /// <summary>
        /// Instantiates a new market by order items.
        /// </summary>
        public MarketByOrderItems()
        {
            for (int i = 0; i < MAX_MARKET_PRICE_ITEM_LIST_SIZE; i++)
            {
                MarketByOrderList.Add(new MarketByOrderItem());
            }
        }

        /// <summary>
        /// Initializes market price item list.
        /// </summary>
        public void Init()
        {
            foreach (MarketByOrderItem mboItem in MarketByOrderList)
            {
                mboItem.Clear();
            }
        }

        /// <summary>
        /// Updates any item that's currently in use.
        /// </summary>
        public void Update()
        {
            foreach (MarketByOrderItem mboItem in MarketByOrderList)
            {
                if (mboItem.IsInUse)
                    mboItem.UpdateFields();
            }
        }

        /// <summary>
        /// Gets storage for a market by order item from the list.
        /// </summary>
        /// <param name="itemName">the item name</param>
        /// <returns>the market by order item</returns>
        public MarketByOrderItem? Get(string itemName)
        {
            // first try to find one with same name and reuse
            foreach (MarketByOrderItem mboItem in MarketByOrderList)
            {
                if (mboItem.IsInUse && mboItem.ItemName != null && mboItem.ItemName.Equals(itemName))
                {
                    return mboItem;
                }
            }

            // next get a new one
            foreach (MarketByOrderItem mboItem in MarketByOrderList)
            {
                if (!mboItem.IsInUse)
                {
                    mboItem.InitFields();
                    mboItem.ItemName = itemName;
                    return mboItem;
                }
            }

            return null;
        }

        /// <summary>
        /// Clears the item information.
        /// </summary>
        public void Clear()
        {
            foreach (MarketByOrderItem mboItem in MarketByOrderList)
            {
                mboItem.Clear();
            }
        }

        /// <summary>
        /// Encodes the market by order response. Returns success if encoding
        /// succeeds or failure if encoding fails.
        /// </summary>
        /// <param name="channel">The channel to send a market by order response to</param>
        /// <param name="itemInfo">The item information</param>
        /// <param name="msgBuf">The message buffer to encode the market by order response into</param>
        /// <param name="isSolicited">The response is solicited if set</param>
        /// <param name="streamId">The stream id of the market by order response</param>
        /// <param name="isStreaming">Flag for streaming or snapshot</param>
        /// <param name="isPrivateStream">the is private stream</param>
        /// <param name="serviceId">The service id of the market by order response</param>
        /// <param name="dictionary">The dictionary used for encoding</param>
        /// <param name="error">Error information in case of encoding failure</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode EncodeResponse(IChannel channel, ItemInfo itemInfo, ITransportBuffer msgBuf, bool isSolicited, int streamId, bool isStreaming, 
            bool isPrivateStream, int serviceId, DataDictionary dictionary, out Error? error)
        {
            error = null;
            if (itemInfo.IsRefreshRequired)
            {
                m_MarketByOrderRefresh.Clear();
                m_MarketByOrderRefresh.Dictionary = dictionary;

                // refresh complete
                m_MarketByOrderRefresh.RefreshComplete = true;


                // service Id
                m_MarketByOrderRefresh.ServiceId = serviceId;
                m_MarketByOrderRefresh.HasServiceId = true;

                // QoS
                m_MarketByOrderRefresh.Qos.IsDynamic = false;
                m_MarketByOrderRefresh.Qos.Timeliness(QosTimeliness.REALTIME);
                m_MarketByOrderRefresh.Qos.Rate(QosRates.TICK_BY_TICK);
                m_MarketByOrderRefresh.HasQos = true;

                // state
                if (isStreaming)
                {
                    m_MarketByOrderRefresh.State.StreamState(StreamStates.OPEN);
                }
                else
                {
                    m_MarketByOrderRefresh.State.StreamState(StreamStates.NON_STREAMING);
                }

                if (isPrivateStream)
                {
                    m_MarketByOrderRefresh.PrivateStream = true;
                }
                m_MarketByOrderRefresh.State.DataState(DataStates.OK);
                m_MarketByOrderRefresh.State.Code(StateCodes.NONE);
                m_MarketByOrderRefresh.State.Text().Data("Item Refresh Completed");

                if (isSolicited)
                {
                    m_MarketByOrderRefresh.Solicited = true;

                    // clear cache for solicited refresh messages.
                    m_MarketByOrderRefresh.ClearCache = true;
                }

                m_MarketByOrderRefresh.StreamId = streamId;
                m_MarketByOrderRefresh.ItemName.Data(itemInfo.ItemName.Data(), itemInfo.ItemName.Position, itemInfo.ItemName.Length);
                m_MarketByOrderRefresh.MboInfo = ((MarketByOrderItem)itemInfo.ItemData!);

                // encode
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
                ret = m_MarketByOrderRefresh.Encode(m_EncodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"MarketByOrderRefresh.Encode() failed"
                    };

                    return ret;
                }
            }
            else
            {
                m_MarketByOrderUpdate.Clear();
                m_MarketByOrderUpdate.Dictionary = dictionary;
                if (isPrivateStream)
                {
                    m_MarketByOrderUpdate.PrivateStream = true;
                }
                m_MarketByOrderUpdate.StreamId = streamId;
                m_MarketByOrderUpdate.MboInfo = ((MarketByOrderItem)itemInfo.ItemData!);

                // encode
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
                ret = m_MarketByOrderUpdate.Encode(m_EncodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"MarketByOrderUpdate.Encode() failed"
                    };

                    return ret;
                }
            }

            return CodecReturnCode.SUCCESS;
        }
    }
}
