/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.PerfTools.Common
{
    public class ItemInfo
    {
        public int ItemId;        // Item's Stream ID
        public ItemData ItemData;        // Holds information about the item's data. This data will be different depending on the domain of the item.
        public int ItemFlags;        // See ItemFlags struct
        public ItemAttributes Attributes;    // Attributes that uniquely identify this item
        public long ClientHandle;
        public long ItemHandle;    // Handle value of this item
        public bool Active;        // True if not closed
        public MarketPriceItem? MarketPriceItem;

        /// <summary>
        /// Instantiates a new item info.
        /// </summary>
        public ItemInfo()
        {
            ItemData = new ItemData();
            Attributes = new ItemAttributes();
        }

        /// <summary>
        /// Clears the item information.
        /// </summary>
        public void Clear()
        {
            ItemFlags = 0;
            ItemId = 0;
            ItemData.Clear();
            Attributes.Clear();
            ClientHandle = 0;
            ItemHandle = 0;
            Active = false;
            MarketPriceItem = null;
        }

        
        public override bool Equals(object? o)
        {
            if (this == o) return true;
            if (o == null || o is not ItemInfo) return false;
            ItemInfo itemInfo = (ItemInfo)o;
            return ClientHandle == itemInfo.ClientHandle && ItemHandle == itemInfo.ItemHandle;
        }

        public override int GetHashCode()
        {
            return (int)(ClientHandle ^ ItemHandle);
        }
    }
}
