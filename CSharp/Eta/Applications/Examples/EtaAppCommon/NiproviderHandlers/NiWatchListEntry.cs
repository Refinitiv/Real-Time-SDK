/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Rdm;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.Example.Common
{
    public class NiWatchListEntry
    {
        public string? ItemName { get; set; }
        public int Type { get; set; }
        public MarketPriceItem? MarketPriceItem { get; set; }
        public MarketByOrderItem? MarketByOrderItem { get; set; }
         
        public override string ToString()
        {
            return "itemName: "
                    + ItemName
                    + ", itemInfo:\n"
                    + ((Type == (int)DomainType.MARKET_PRICE) ? MarketPriceItem : MarketByOrderItem);
        }
    }
}
