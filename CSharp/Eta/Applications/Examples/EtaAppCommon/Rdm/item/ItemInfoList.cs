/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System.Collections;
using System.Collections.Generic;

namespace LSEG.Eta.Example.Common
{
    public class ItemInfoList : IEnumerable<ItemInfo>
    {
        private static readonly int MAX_ITEM_LIST_SIZE = ProviderDirectoryHandler.OPEN_LIMIT * ProviderSession.NUM_CLIENT_SESSIONS;
        private List<ItemInfo> m_ItemInfoList = new List<ItemInfo>(MAX_ITEM_LIST_SIZE);

        /// <summary>
        /// Instantiates a new item info list.
        /// </summary>
        public ItemInfoList()
        {
            for (int i = 0; i < MAX_ITEM_LIST_SIZE; i++)
            {
                m_ItemInfoList.Add(new ItemInfo());
            }
        }

        /// <summary>
        /// Retrieves an item info given the item name and domain.
        /// </summary>
        /// <param name="name">Itemname to search iteminfo for</param>
        /// <param name="domainType">Domain type to search iteminfo for</param>
        /// <param name="isPrivateStream">Private stream attribute to search iteminfo for</param>
        /// <returns><see cref="ItemInfo"/> associated with the search attributes</returns>
        public ItemInfo? Get(Buffer name, int domainType, bool isPrivateStream)
        {
            foreach (ItemInfo itemInfo in m_ItemInfoList)
            {
                if (itemInfo.DomainType == domainType && name.Equals(itemInfo.ItemName)
                    && itemInfo.IsPrivateStream == isPrivateStream)
                    return itemInfo;
            }

            return null;
        }

        /// <summary>
        /// Finds an item information associated with the item request.
        /// </summary>
        /// <param name="channel">The channel</param>
        /// <param name="itemName">Itemname to search iteminfo for</param>
        /// <param name="domainType">Domain type to search iteminfo for</param>
        /// <param name="isPrivateStream">Private stream attribute to search iteminfo for</param>
        /// <returns><see cref="ItemInfo"/> associated with the search attributes or get a new one from the list.</returns>
        public ItemInfo? Get(IChannel channel, Buffer itemName, int domainType, bool isPrivateStream)
        {
            /* first check for existing item */
            ItemInfo? itemInfo = Get(itemName, domainType, isPrivateStream);

            /* if no existing item, get new one */
            if (itemInfo is null)
            {
                foreach (ItemInfo tmpItemInfo in m_ItemInfoList)
                {
                    if (tmpItemInfo.InterestCount == 0)
                    {
                        // copy item name buffer
                        BufferHelper.CopyBuffer(itemName, tmpItemInfo.ItemName);
                        tmpItemInfo.DomainType = domainType;
                        tmpItemInfo.IsPrivateStream = isPrivateStream;
                        itemInfo = tmpItemInfo;
                        break;
                    }
                }
            }

            return itemInfo;
        }

        public IEnumerator<ItemInfo> GetEnumerator()
        {
            return m_ItemInfoList.GetEnumerator();
        }

        /// <summary>
        /// Updates any item information that's currently in use.
        /// </summary>
        public void Update()
        {
            foreach (ItemInfo itemInfo in m_ItemInfoList)
            {
                if (itemInfo.InterestCount > 0)
                {
                    switch (itemInfo.DomainType)
                    {
                        case (int)Rdm.DomainType.MARKET_PRICE:
                            if(itemInfo.ItemData != null)
                                ((MarketPriceItem)itemInfo.ItemData).UpdateFields();
                            break;
                        case (int)Rdm.DomainType.MARKET_BY_ORDER:
                            if (itemInfo.ItemData != null)
                                ((MarketByOrderItem)itemInfo.ItemData).UpdateFields();
                            break;
                        case (int)Rdm.DomainType.MARKET_BY_PRICE:
                            if (itemInfo.ItemData != null)
                                ((MarketByPriceItem)itemInfo.ItemData).UpdateFields();
                            break;
                        case (int)Rdm.DomainType.SYMBOL_LIST:
                            break;
                        default:
                            System.Console.WriteLine("Unknown domain");
                            break;
                    }
                }
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        /// <summary>
        /// Initializes item information list.
        /// </summary>
        public void Init()
        {
            foreach (ItemInfo itemInfo in m_ItemInfoList)
            {
                itemInfo.Clear();
            }
        }

        /**
         * clears item information list.
         */
        public void Clear()
        {
            foreach (ItemInfo itemInfo in m_ItemInfoList)
            {
                itemInfo.Clear();
            }
        }
    }
}
