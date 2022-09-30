/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Rdm;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Refinitiv.Eta.Example.Common
{
    public class NiStreamIdWatchList : IEnumerable
    {
        private Dictionary<int, NiWatchListEntry> watchList = new Dictionary<int, NiWatchListEntry>();

        // starting stream id for marketPrice / marketByOrder streams
        // 1 - login, -1 - directory
        private int nextStreamId = -2;

        public int Add(int domainType, string itemName)
        {
            foreach (var entry in watchList)
            {
                if (entry.Value.ItemName!.Equals(itemName))
                {
                    return entry.Key;
                }
            }

            NiWatchListEntry wle = new NiWatchListEntry();
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

            int thisStreamId = nextStreamId--;
            watchList.Add(thisStreamId, wle);
            return thisStreamId;
        }

        public bool Remove(int streamId)
        {
            return watchList.ContainsKey(streamId) && watchList.Remove(streamId);
        }

        public NiWatchListEntry Get(int streamId)
        {
            return watchList[streamId];
        }

        public void clear()
        {
            watchList.Clear();
        }
        public IEnumerator GetEnumerator()
        {
            return watchList.GetEnumerator();
        }
    }
}
