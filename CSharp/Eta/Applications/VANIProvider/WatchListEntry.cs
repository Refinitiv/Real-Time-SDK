/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;

namespace LSEG.Eta.ValueAdd.VANiProvider
{
    /// <summary>
    /// WatchList entry used by <see cref="StreamIdWatchList"/>
    /// </summary>
    public class WatchListEntry
    {
        public string? ItemName { get; set; }
        public int Type { get; set; }
        public MarketPriceItem? MarketPriceItem { get; set; }
        public MarketByOrderItem? MarketByOrderItem { get; set; }

        public override string ToString()
        {
            return $"itemName: {ItemName}, itemInfo:\n {((Type == (int)DomainType.MARKET_PRICE) ? MarketPriceItem : MarketByOrderItem)}";
        }
    }
}
