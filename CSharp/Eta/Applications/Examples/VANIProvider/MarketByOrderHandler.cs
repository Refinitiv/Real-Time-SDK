/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.Transports;

namespace LSEG.Eta.ValueAdd.VANiProvider
{
    /// <summary>
    /// This is the market by order handler for the ETA Value Add NIProvider application.  
    /// It provides methods to encode and send refreshes and updates, as well as closing streams.
    /// </summary>
    public class MarketByOrderHandler
    {
        public static int TRANSPORT_BUFFER_SIZE_REQUEST = 1024;
        public static int TRANSPORT_BUFFER_SIZE_CLOSE = 1024;

        private int m_DomainType;

        private MarketByOrderRefresh m_MarketByOrderRefresh;
        private MarketByOrderUpdate m_MarketByOrderUpdate;
        private MarketByOrderClose m_CloseMessage;

        private StreamIdWatchList m_WatchList;

        // reusable variables used for encoding
        protected FieldList m_FieldList = new FieldList();
        protected FieldEntry m_FieldEntry = new FieldEntry();
        private EncodeIterator m_EncodeIter = new EncodeIterator();

        private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();

        /// <summary>
        /// Initializes a MarketByOrderHandler instance
        /// </summary>
        /// <param name="watchList"><see cref="StreamIdWatchList"/> used by MarketByOrderHandler</param>
        /// <param name="dictionary"><see cref="DataDictionary"/> instance used by MarketByOrderHandler</param>
        public MarketByOrderHandler(StreamIdWatchList watchList, DataDictionary dictionary) : this(watchList, (int)DomainType.MARKET_BY_ORDER, dictionary) { }

        /// <summary>
        /// Closes all streams associated with MarketByOrder domain
        /// </summary>
        /// <param name="chnl"><see cref="ReactorChannel"/> associated with the current connection</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating the status of the operation</returns>
        public ReactorReturnCode CloseStreams(ReactorChannel chnl)
        {
            var iter = m_WatchList.GetEnumerator();
            HashSet<int> itemsToRemove = new HashSet<int>();
            while (iter.MoveNext())
            {
                var entry = iter.Current;

                if (entry.Value.Type != m_DomainType)
                {
                    // this entry is from a different domainType, skip
                    continue;
                }

                if (CloseStream(chnl, entry.Key, out ReactorErrorInfo? errorInfo) < ReactorReturnCode.SUCCESS)
                {
                    Console.WriteLine($"Failed to close ReactorChannel {chnl}: {(errorInfo != null ? errorInfo.Error.Text : "")}");
                }
                itemsToRemove.Add(entry.Key);
            }
            m_WatchList.RemoveAll(itemsToRemove);   
            m_WatchList.Clear();

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends item refreshes for the specified item names
        /// </summary>
        /// <param name="chnl"><see cref="ReactorChannel"/> associated with the current connection</param>
        /// <param name="itemNames">the list of items for which refreshes will be sent</param>
        /// <param name="serviceId">the id of the service provided by the NiProvider instance</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> structure that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating the status of the operation</returns>
        public ReactorReturnCode SendItemRefreshes(ReactorChannel chnl, List<string> itemNames, int serviceId, out ReactorErrorInfo? errorInfo)
        {
            if (itemNames == null || itemNames.Count == 0)
            {
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }             

            GenerateRefreshAndUpdate(serviceId);

            return SendRefreshes(chnl, itemNames, out errorInfo);
        }

        public ReactorReturnCode SendItemUpdates(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            int itemCount = 0;
            var iter = m_WatchList.GetEnumerator();
            while (iter.MoveNext())
            {
                var entry = iter.Current;
                WatchListEntry wle = entry.Value;
                if (wle.Type != m_DomainType)
                {
                    /* this entry is from a different domainType, skip */
                    continue;
                }
                /* update fields */
                wle.MarketByOrderItem!.UpdateFields();

                m_MarketByOrderUpdate.StreamId = entry.Key;
                m_MarketByOrderUpdate.ItemName.Data(wle.ItemName);
                m_MarketByOrderUpdate.MboInfo = wle.MarketByOrderItem;

                ret = EncodeAndSendContent(chnl, m_MarketByOrderUpdate, wle, false, out errorInfo);
                if (ret < ReactorReturnCode.SUCCESS)
                    return ret;
                itemCount++;
            }

            if (itemCount > 0)
            {
                Console.WriteLine($"Sent {itemCount} MarketByOrder items.");
            }

            errorInfo = null;
            return ret;
        }

        protected MarketByOrderHandler(StreamIdWatchList watchList, int domainType, DataDictionary dictionary)
        {
            m_WatchList = watchList;
            m_DomainType = domainType;
            m_MarketByOrderRefresh = CreateMarketByOrderRefresh();
            m_MarketByOrderRefresh.Dictionary = dictionary;
            m_MarketByOrderUpdate = CreateMarketByOrderUpdate();
            m_MarketByOrderUpdate.Dictionary = dictionary;
            m_CloseMessage = new MarketByOrderClose();
        }

        protected MarketByOrderRefresh CreateMarketByOrderRefresh()
        {
            return new MarketByOrderRefresh();
        }

        protected MarketByOrderUpdate CreateMarketByOrderUpdate()
        {
            return new MarketByOrderUpdate();
        }

        private ReactorReturnCode CloseStream(ReactorChannel chnl, int streamId, out ReactorErrorInfo? errorInfo)
        {
            if (chnl.State == ReactorChannelState.UP)
            {
                //get a buffer for the item close
                ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out errorInfo);
                if (msgBuf == null)
                {
                    return ReactorReturnCode.FAILURE;
                }

                //encode item close
                m_CloseMessage.Clear();
                m_CloseMessage.StreamId = streamId;
                m_EncodeIter.Clear();
                m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

                CodecReturnCode ret = m_CloseMessage.Encode(m_EncodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = $"EncodeMarketByOrderClose(): Failed <{ret.GetAsString()}>";
                    Console.WriteLine(errorInfo.Error.Text);
                }
                return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private void GenerateRefreshAndUpdate(int serviceId)
        {
            //refresh complete
            m_MarketByOrderRefresh.RefreshComplete = true;

            //service Id
            m_MarketByOrderRefresh.HasServiceId = true;
            m_MarketByOrderRefresh.ServiceId = serviceId;            
            m_MarketByOrderUpdate.ServiceId = serviceId;
            m_MarketByOrderUpdate.HasServiceId = true;

            //QoS
            m_MarketByOrderRefresh.Qos.IsDynamic = false;
            m_MarketByOrderRefresh.Qos.Timeliness(QosTimeliness.REALTIME);
            m_MarketByOrderRefresh.Qos.Rate(QosRates.TICK_BY_TICK);
            m_MarketByOrderRefresh.HasQos = true;

            //state
            m_MarketByOrderRefresh.State.StreamState(StreamStates.OPEN);
            m_MarketByOrderRefresh.State.DataState(DataStates.OK);
            m_MarketByOrderRefresh.State.Code(StateCodes.NONE);
            m_MarketByOrderRefresh.State.Text().Data("Item Refresh Completed");
        }

        private ReactorReturnCode SendRefreshes(ReactorChannel chnl, List<string> itemNames, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            WatchListEntry? wlEntry;
            foreach (string itemName in itemNames)
            {
                int streamId = m_WatchList.Add(m_DomainType, itemName);

                m_MarketByOrderRefresh.ItemName.Data(itemName);
                m_MarketByOrderRefresh.StreamId = streamId;

                wlEntry = m_WatchList!.Get(streamId);

                if (wlEntry == null)
                {
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = "Non existing stream id: " + streamId;
                    errorInfo.Error.ErrorId = TransportReturnCode.FAILURE;
                    return ReactorReturnCode.FAILURE;
                }

                m_MarketByOrderRefresh.MboInfo = wlEntry.MarketByOrderItem!;


                ret = EncodeAndSendContent(chnl, m_MarketByOrderRefresh, m_WatchList.Get(streamId)!, true, out errorInfo);
                if (ret < ReactorReturnCode.SUCCESS)
                    return ret;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode EncodeAndSendContent(ReactorChannel chnl, MarketByOrderBase marketContent, WatchListEntry wle, bool isRefresh, out ReactorErrorInfo? errorInfo)
        {
            //get a buffer for the item request
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out errorInfo);

            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            m_EncodeIter.Clear();
            m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            CodecReturnCode ret = marketContent.Encode(m_EncodeIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"MarketByOrderResponse.Encode failed: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

    }
}
