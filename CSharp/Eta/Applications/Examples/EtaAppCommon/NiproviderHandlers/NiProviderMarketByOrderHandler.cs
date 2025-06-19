/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.Example.Common
{
    public class NiProviderMarketByOrderHandler : MarketHandlerBase
    {
        public static int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;

        private int domainType;

        private MarketByOrderRefresh marketByOrderRefresh;
        private MarketByOrderUpdate marketByOrderUpdate;

        /// <summary>
        /// Instantiates a new NIProvider MarketByOrder handler.
        /// </summary>
        /// <param name="watchList">The watch list.</param>
        /// <param name="dictionary">The dictionary instance that will be used by current NIProvider.</param>
        public NiProviderMarketByOrderHandler(NiStreamIdWatchList watchList, DataDictionary dictionary) : this(watchList, (int)DomainType.MARKET_BY_ORDER, dictionary)
        { }

        protected NiProviderMarketByOrderHandler(NiStreamIdWatchList watchList, int domainType, DataDictionary dictionary)
        {
            this.watchList = watchList;
            this.domainType = domainType;
            marketByOrderRefresh = CreateMarketByOrderRefresh();
            marketByOrderRefresh.Dictionary = dictionary;
            marketByOrderUpdate = CreateMarketByOrderUpdate();
            marketByOrderUpdate.Dictionary = dictionary;
        }

        protected MarketByOrderRefresh CreateMarketByOrderRefresh()
        {
            return new MarketByOrderRefresh();
        }

        protected MarketByOrderUpdate CreateMarketByOrderUpdate()
        {
            return new MarketByOrderUpdate();
        }

        protected override void GenerateRefreshAndUpdate(Service serviceInfo)
        {
            //refresh complete
            marketByOrderRefresh.RefreshComplete = true;

            //service Id
            marketByOrderRefresh.HasServiceId = true;
            marketByOrderRefresh.ServiceId = serviceInfo.ServiceId;
            marketByOrderUpdate.HasServiceId = true;
            marketByOrderUpdate.ServiceId = serviceInfo.ServiceId;

            //QoS
            marketByOrderRefresh.Qos.IsDynamic = false;
            marketByOrderRefresh.Qos.Timeliness(QosTimeliness.REALTIME);
            marketByOrderRefresh.HasQos = true;
            marketByOrderRefresh.Qos.Rate(QosRates.TICK_BY_TICK);

            //state
            marketByOrderRefresh.State.StreamState(StreamStates.OPEN);
            marketByOrderRefresh.State.DataState(DataStates.OK);
            marketByOrderRefresh.State.Code(StateCodes.NONE);
            marketByOrderRefresh.State.Text().Data("Item Refresh Completed");
        }

        protected override TransportReturnCode SendRefreshes(ChannelSession chnl, List<string> itemNames, out Error? error)
        {
            TransportReturnCode ret;
            NiWatchListEntry? wlEntry;
            foreach (string itemName in itemNames)
            {
                int streamId = watchList!.Add(domainType, itemName);

                marketByOrderRefresh.ItemName.Data(itemName);
                marketByOrderRefresh.StreamId = streamId;

                wlEntry = watchList!.Get(streamId);

                if (wlEntry == null)
                {
                    error = new Error()
                    {
                        Text = "Non existing stream id: " + streamId
                    };
                    return TransportReturnCode.FAILURE;
                }

                marketByOrderRefresh.MboInfo = wlEntry.MarketByOrderItem!;

                ret = EncodeAndSendContent(chnl, marketByOrderRefresh, out error);
                if (ret < TransportReturnCode.SUCCESS)
                    return ret;
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes and sends item updates for MarketPrice domain.
        /// </summary>
        /// <param name="chnl">The channel to send a update to.</param>
        /// <param name="error">Populated if an error occurs.</param>
        /// <returns>success if item update can be made, can be encoded and sent successfully. 
        /// Failure if encoding/sending update failed.</returns>
        public TransportReturnCode SendItemUpdates(ChannelSession chnl, out Error? error)
        {
            TransportReturnCode ret = TransportReturnCode.SUCCESS;
            var enumer = watchList!.GetEnumerator();
            while (enumer.MoveNext())
            {
                KeyValuePair<int, NiWatchListEntry> entry = (KeyValuePair<int, NiWatchListEntry>)enumer.Current;
                if (entry.Value.Type != domainType)
                {
                    //this entry is from a different domainType, skip
                    continue;
                }
                entry.Value.MarketByOrderItem!.UpdateFields();

                marketByOrderUpdate.StreamId = entry.Key;
                marketByOrderUpdate.ItemName.Data(entry.Value.ItemName);
                marketByOrderUpdate.MboInfo = entry.Value.MarketByOrderItem;

                ret = EncodeAndSendContent(chnl, marketByOrderUpdate, out error);
                if (ret < TransportReturnCode.SUCCESS)
                    return ret;
            }

            error = null;
            return ret;
        }

    }
}
