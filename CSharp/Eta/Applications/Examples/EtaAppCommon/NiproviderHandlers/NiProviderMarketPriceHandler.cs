/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
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
    /// <summary>
    /// This is the market price handler for the ETA NIProvider application. 
    /// It provides methods to encode and send refreshes and updates, as well as close streams.
    /// </summary>
    public class NiProviderMarketPriceHandler : MarketHandlerBase
    {       
        private int domainType;
        private MarketPriceRefresh marketPriceRefresh;
        private MarketPriceUpdate marketPriceUpdate;

        /// <summary>
        /// Instantiates a new market price handler.
        /// </summary>
        /// <param name="watchList">The watch list used by NiProvider</param>
        /// <param name="dictionary">The dictionary used by NiProvider</param>
        public NiProviderMarketPriceHandler(NiStreamIdWatchList watchList, DataDictionary dictionary) : this(watchList, (int)DomainType.MARKET_PRICE, dictionary) { }

        protected NiProviderMarketPriceHandler(NiStreamIdWatchList watchList, int domainType, DataDictionary dictionary)
        {
            this.watchList = watchList;
            this.domainType = domainType;
            marketPriceRefresh = CreateMarketPriceRefresh();
            marketPriceRefresh.DataDictionary = dictionary;
            marketPriceUpdate = CreateMarketPriceUpdate();
            marketPriceUpdate.DataDictionary = dictionary;
            closeMessage = new MarketPriceClose();
        }

        protected MarketPriceRefresh CreateMarketPriceRefresh()
        {
            return new MarketPriceRefresh();
        }

        protected MarketPriceUpdate CreateMarketPriceUpdate()
        {
            return new MarketPriceUpdate();
        }

        protected override void GenerateRefreshAndUpdate(Service serviceInfo)
        {
            marketPriceRefresh.RefreshComplete = true;

            //service Id
            marketPriceRefresh.HasServiceId = true;
            marketPriceRefresh.ServiceId = serviceInfo.ServiceId;
            marketPriceUpdate.HasServiceId = true;
            marketPriceUpdate.ServiceId = serviceInfo.ServiceId;            

            //QoS
            marketPriceRefresh.Qos.IsDynamic = false;
            marketPriceRefresh.Qos.Timeliness(QosTimeliness.REALTIME);
            marketPriceRefresh.HasQos = true;
            marketPriceRefresh.Qos.Rate(QosRates.TICK_BY_TICK);            

            //state
            marketPriceRefresh.State.StreamState(StreamStates.OPEN);
            marketPriceRefresh.State.DataState(DataStates.OK);
            marketPriceRefresh.State.Code(StateCodes.NONE);
            marketPriceRefresh.State.Text().Data("Item Refresh Completed");
        }

        protected override TransportReturnCode SendRefreshes(ChannelSession chnl, List<string> itemNames, out Error? error)
        {
            TransportReturnCode ret;
            NiWatchListEntry? wlEntry;
            foreach (string itemName in itemNames)
            {
                int streamId = watchList!.Add(domainType, itemName);

                marketPriceRefresh.ItemName.Data(itemName);
                marketPriceRefresh.StreamId = streamId;

                wlEntry = watchList!.Get(streamId);

                if (wlEntry == null)
                {
                    error = new Error()
                    {
                        Text = "Non existing stream id: " + streamId
                    };
                    return TransportReturnCode.FAILURE;
                }

                marketPriceRefresh.MarketPriceItem = wlEntry.MarketPriceItem!;

                ret = EncodeAndSendContent(chnl, marketPriceRefresh, out error);
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
                entry.Value.MarketPriceItem!.UpdateFields();

                marketPriceUpdate.StreamId = entry.Key;
                marketPriceUpdate.ItemName.Data(entry.Value.ItemName);
                marketPriceUpdate.MarketPriceItem = entry.Value.MarketPriceItem;

                ret = EncodeAndSendContent(chnl, marketPriceUpdate, out error);
                if (ret < TransportReturnCode.SUCCESS)
                    return ret;
            }

            error = null;
            return ret;
        }
    }
}
