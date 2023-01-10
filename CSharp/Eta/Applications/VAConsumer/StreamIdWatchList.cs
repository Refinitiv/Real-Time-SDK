/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Collections;

using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;


namespace LSEG.Eta.ValueAdd.Consumer
{
    /// <summary>
    /// This is a hash map based WatchList for quick lookup of item states when
    /// response is received.
    /// </summary>
    ///
    /// It is a map of (stream id -> item states). Initial
    /// state for a stream when request is sent is unspecified. State is updated with
    /// state from status and refresh message.
    public class StreamIdWatchList : IEnumerable<KeyValuePair<Int32, StreamIdWatchList.WatchListEntry>>
    {
        // This implementation doesn't use Cache's PayloadItem
        private Dictionary<int, WatchListEntry> m_WatchList = new();

        // starting stream id for marketPrice / marketByOrder streams
        // 1 - login, 2- directory, 3-field dictionary, 4 - enum dictionary
        private int m_NextStreamId = 5;

        public bool IsEmpty { get => m_WatchList.Count == 0; }

        public int Add(int domainType, string itemName, bool isPrivateStream = false)
        {
            // check if item is already in list before adding
            foreach (var entry in m_WatchList)
            {
                if (entry.Value.ItemName.Equals(itemName))
                {
                    // return stream id for matching item
                    return entry.Key;
                }
            }

            // add new entry
            WatchListEntry wle = new WatchListEntry()
            {
                ItemName = itemName,
                DomainType = domainType,
                IsPrivateStream = isPrivateStream
            };

            if (domainType == (int)DomainType.MARKET_PRICE)
            {
                wle.MarketPriceItem = new MarketByPriceItem();
                wle.MarketPriceItem.InitFields();
            }
            else if (domainType == (int)DomainType.MARKET_BY_ORDER)
            {
                wle.MarketByOrderItem = new MarketByOrderItem();
                wle.MarketByOrderItem.InitFields();
            }

            var thisStreamId = m_NextStreamId++;
            m_WatchList.Add(thisStreamId, wle);

            return thisStreamId;
        }

        public WatchListEntry? Get(int streamId)
        {
            if (m_WatchList.TryGetValue(streamId, out var entry))
                return entry;
            else
                return null;
        }

        public int GetFirstItem(Codec.Buffer mpItemName)
        {
            foreach (var entry in m_WatchList)
            {
                WatchListEntry wle = entry.Value;
                State itemState = wle.ItemState;
                if (itemState.DataState() == DataStates.OK
                        && itemState.StreamState() == StreamStates.OPEN)
                {
                    mpItemName.Data(wle.ItemName);
                    return entry.Key;
                }
            }

            /* no suitable items were found */
            mpItemName.Clear();

            return 0;
        }


        public void Clear()
        {
            m_WatchList.Clear();
        }

        IEnumerator<KeyValuePair<int, StreamIdWatchList.WatchListEntry>>
            IEnumerable<KeyValuePair<int, StreamIdWatchList.WatchListEntry>>.GetEnumerator()
        {
            return m_WatchList.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_WatchList.GetEnumerator();
        }

        /// <summary>
        /// Watch list entry used by <see cref="StreamIdWatchList"/>.
        /// </summary>
        public class WatchListEntry
        {
            public string ItemName = String.Empty;
            public int DomainType;
            public bool IsPrivateStream = false;
            public MarketByPriceItem? MarketPriceItem;
            public MarketByOrderItem? MarketByOrderItem;
            public State ItemState = new();

            public override string ToString()
            {
                return "itemName: "
                        + ItemName
                        + ", itemInfo:\n"
                        + ((DomainType == (int)Eta.Rdm.DomainType.MARKET_PRICE)
                                ? this.MarketPriceItem
                                : this.MarketByOrderItem);
            }
        }

        internal bool Remove(int streamId)
        {
            return m_WatchList.Remove(streamId);
        }
    }
}
