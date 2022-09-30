/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Example.Common;
using Refinitiv.Eta.Rdm;
using Refinitiv.Eta.ValueAdd.Reactor;

namespace Refinitiv.Eta.ValueAdd.VANiProvider
{
    /// <summary>
    /// This is the market price handler for the ETA NIProvider application. 
    /// It provides methods to encode and send refreshes and updates, as well as close streams. 
    /// </summary>
    public class MarketPriceHandler
    {
        public static int TRANSPORT_BUFFER_SIZE_MESSAGE = 1024;
        public static int TRANSPORT_BUFFER_SIZE_CLOSE = 1024;

        private int m_DomainType;

        private MarketPriceRefresh m_MarketPriceRefresh;
        private MarketPriceUpdate m_MarketPriceUpdate;
        private MarketPriceClose m_CloseMessage;

        private StreamIdWatchList m_WatchList;

        // reusable variables used for encoding
        protected FieldList m_FieldList = new FieldList();
        protected FieldEntry m_FieldEntry = new FieldEntry();
        private EncodeIterator m_EncodeIter = new EncodeIterator();

        private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();

        protected MarketPriceHandler(StreamIdWatchList watchList, int domainType, DataDictionary dictionary)
        {
            m_WatchList = watchList;
            m_DomainType = domainType;
            m_MarketPriceRefresh = CreateMarketPriceRefresh();
            m_MarketPriceRefresh.DataDictionary = dictionary;
            m_MarketPriceUpdate = CreateMarketPriceUpdate();
            m_MarketPriceUpdate.DataDictionary = dictionary;
            m_CloseMessage = new MarketPriceClose();
        }

        /// <summary>
        /// Initializes a MarketPriceHandler instance
        /// </summary>
        /// <param name="watchList"><see cref="StreamIdWatchList"/> used by MarketPriceHandler</param>
        /// <param name="dictionary"><see cref="DataDictionary"/> instance used by MarketPriceHandler</param>
        public MarketPriceHandler(StreamIdWatchList watchList, DataDictionary dictionary) : this(watchList, (int)DomainType.MARKET_PRICE, dictionary) {}        

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

        /// <summary>
        /// Sends updates for the items
        /// </summary>
        /// <param name="chnl"><see cref="ReactorChannel"/> associated with the current connection</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> structure that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating the status of the operation</returns>
        public ReactorReturnCode SendItemUpdates(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            int itemCount = 0;
            var iter = m_WatchList.GetEnumerator();
            while (iter.MoveNext())
            {
                var entry = iter.Current;
                if (entry.Value.Type != m_DomainType)
                {
                    //this entry is from a different domainType, skip
                    continue;
                }
                //update fields
                entry.Value.MarketPriceItem!.UpdateFields();

                m_MarketPriceUpdate.StreamId = entry.Key;
                m_MarketPriceUpdate.ItemName.Data(entry.Value.ItemName);
                m_MarketPriceUpdate.MarketPriceItem = entry.Value.MarketPriceItem;

                ret = EncodeAndSendContent(chnl, m_MarketPriceUpdate, entry.Value, false, out errorInfo);
                if (ret < ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }
                itemCount++;
            }

            if (itemCount > 0)
            {
                Console.WriteLine($"Sent {itemCount} MarketPrice items.");
            }

            errorInfo = null;
            return ret;
        }

        /// <summary>
        /// Closes all streams associated with MarketPrice domain
        /// </summary>
        /// <param name="chnl"><see cref="ReactorChannel"/> associated with the current connection</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating the status of the operation</returns>
        public ReactorReturnCode CloseStreams(ReactorChannel chnl)
        {
            ReactorReturnCode ret;
            var iter = m_WatchList.GetEnumerator();
            HashSet<int> itemsToRemove = new HashSet<int>();
            while (iter.MoveNext())
            {
                var entry = iter.Current;
                if (entry.Value.Type != m_DomainType)
                {
                    //this entry is from a different domainType, skip
                    continue;
                }

                ret = CloseStream(chnl, entry.Key, out ReactorErrorInfo? errorInfo);
                if (ret != ReactorReturnCode.SUCCESS && errorInfo != null)
                {
                    Console.WriteLine($"Failed to close stream with id {entry.Key}: {errorInfo.Error.Text}");
                }
                itemsToRemove.Add(entry.Key);
            }
            m_WatchList.RemoveAll(itemsToRemove);
            return ReactorReturnCode.SUCCESS;
        }

        protected MarketPriceRefresh CreateMarketPriceRefresh()
        {
            return new MarketPriceRefresh();
        }

        protected MarketPriceUpdate CreateMarketPriceUpdate()
        {
            return new MarketPriceUpdate();
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
                m_CloseMessage.DomainType = m_DomainType;
                m_EncodeIter.Clear();
                m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

                CodecReturnCode ret = m_CloseMessage.Encode(m_EncodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine($"Encode MarketPriceClose failed: <{ret.GetAsString()}>");
                }
                return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private void GenerateRefreshAndUpdate(int serviceId)
        {
            //refresh complete
            m_MarketPriceRefresh.RefreshComplete = true;

            //service Id
            m_MarketPriceRefresh.ServiceId = serviceId;
            m_MarketPriceRefresh.HasServiceId = true;
            m_MarketPriceUpdate.ServiceId = serviceId;
            m_MarketPriceUpdate.HasServiceId = true;

            //QoS
            m_MarketPriceRefresh.Qos.IsDynamic = false;
            m_MarketPriceRefresh.Qos.Timeliness(QosTimeliness.REALTIME);
            m_MarketPriceRefresh.Qos.Rate(QosRates.TICK_BY_TICK);
            m_MarketPriceRefresh.HasQos = true;

            //state
            m_MarketPriceRefresh.State.StreamState(StreamStates.OPEN);
            m_MarketPriceRefresh.State.DataState(DataStates.OK);
            m_MarketPriceRefresh.State.Code(StateCodes.NONE);
            m_MarketPriceRefresh.State.Text().Data("Item Refresh Completed");
        }

        private ReactorReturnCode SendRefreshes(ReactorChannel chnl, List<string> itemNames, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;
            foreach (string itemName in itemNames)
            {
                int streamId = m_WatchList.Add(m_DomainType, itemName);

                m_MarketPriceRefresh.ItemName.Data(itemName);
                m_MarketPriceRefresh.StreamId = streamId;
                m_MarketPriceRefresh.MarketPriceItem = m_WatchList.Get(streamId).MarketPriceItem!;

                ret = EncodeAndSendContent(chnl, m_MarketPriceRefresh, m_WatchList.Get(streamId), true, out errorInfo);
                if (ret < ReactorReturnCode.SUCCESS)
                    return ret;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode EncodeAndSendContent(ReactorChannel chnl, MarketPriceBase marketPriceContent, WatchListEntry wle, bool isRefresh, out ReactorErrorInfo? errorInfo)
        {
            //get a buffer for the item refresh/update
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_MESSAGE, false, out errorInfo);

            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            m_EncodeIter.Clear();
            m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            CodecReturnCode ret = marketPriceContent.Encode(m_EncodeIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"MarketPriceResponse.Encode failed: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

       
    }
}
