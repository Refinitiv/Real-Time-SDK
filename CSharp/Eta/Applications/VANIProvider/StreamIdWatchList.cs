/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using System.Collections;

namespace LSEG.Eta.ValueAdd.VANiProvider
{
    /// <summary>
    /// This is a hash map based WatchList for quick lookup of items and their
    /// associated streamId and domainType when sending updates or reissuing refreshes. 
    /// It is a map of (streamId -> item info).
    /// </summary>
    public class StreamIdWatchList : IEnumerable<KeyValuePair<int, WatchListEntry>>
    {
        private Dictionary<int, WatchListEntry> watchList = new Dictionary<int, WatchListEntry>();

        // starting stream id for marketPrice / marketByOrder streams
        // 1 - login, -1 - directory
        private int m_NextStreamId = -2;

        public int Add(int domainType, string itemName)
        {
            // check if item is already in list before adding
            var enumerator = watchList.GetEnumerator();
            while (enumerator.MoveNext())
            {
                var entry = enumerator.Current;
                if (entry.Value.ItemName!.Equals(itemName) &&
                        entry.Value.Type == domainType)
                {
                    // return stream id for matching item
                    return entry.Key;
                }
            }

            // add new entry
            WatchListEntry wle = new WatchListEntry();
            wle.ItemName = itemName;
            wle.Type = domainType;

            if (domainType == (int)DomainType.MARKET_PRICE)
            {
                wle.MarketPriceItem = new MarketPriceItem();
                wle.MarketPriceItem.InitFields();
            }
            else if (domainType == (int)DomainType.MARKET_BY_ORDER)
            {
                wle.MarketByOrderItem = new MarketByOrderItem();
                wle.MarketByOrderItem.InitFields();
            }

            int thisStreamId = m_NextStreamId--;
            watchList.Add(thisStreamId, wle);
            return thisStreamId;
        }

        public void Remove(int key)
        {
            watchList.Remove(key);
        }

        public void RemoveAll(HashSet<int> keys)
        {
            foreach (var key in keys)
                watchList.Remove(key);
        }

        public WatchListEntry Get(int streamId)
        {
            return watchList[streamId];
        }

        public void Clear()
        {
            watchList.Clear();
        }

        public IEnumerator<KeyValuePair<int, WatchListEntry>> GetEnumerator()
        {
            return watchList.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return watchList.GetEnumerator();
        }
    }
}
