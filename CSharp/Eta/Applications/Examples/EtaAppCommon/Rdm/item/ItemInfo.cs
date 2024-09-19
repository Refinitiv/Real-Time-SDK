/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// Item information.
    /// </summary>
    public class ItemInfo
    {
        public bool IsRefreshRequired { get; set; }
        public Buffer ItemName { get; private set; }
        public int InterestCount { get; set; }
        public int DomainType { get; set; }
        public bool IsPrivateStream { get; set; }

        /// <summary>
        /// Gets or sets information about the item's data.
        /// </summary>
        public object? ItemData { get; set; }

        public ItemInfo()
        {
            IsRefreshRequired = true;
            ItemName = new Buffer();
            InterestCount = 0;
            DomainType = 0;
            IsPrivateStream = false;
        }

        /// <summary>
        /// Clears item information
        /// </summary>
        public void Clear()
        {
            switch (DomainType)
            {
                case (int)Rdm.DomainType.MARKET_PRICE:
                    if(ItemData != null)
                        ((MarketPriceItem)ItemData).Clear();
                    break;
                case (int)Rdm.DomainType.MARKET_BY_ORDER:
                    if (ItemData != null)
                        ((MarketByOrderItem)ItemData).Clear();
                    break;
                case (int)Rdm.DomainType.MARKET_BY_PRICE:
                    if (ItemData != null)
                        ((MarketByPriceItem)ItemData).Clear();
                    break;
                case (int)Rdm.DomainType.SYMBOL_LIST:
                    break;
                default:
                    break;
            }

            IsRefreshRequired = true;
            ItemName.Clear();
            InterestCount = 0;
            DomainType = 0;
            IsPrivateStream = false;
        }
    }
}
