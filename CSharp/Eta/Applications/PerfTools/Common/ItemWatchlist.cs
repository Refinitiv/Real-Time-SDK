/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// Item requests list
    /// </summary>
    public class ItemWatchlist
    {
        private List<ItemInfo> m_EntryList;
        private int m_Index;

        /// <summary>
        /// Instantiates a new item watchlist
        /// </summary>
        /// <param name="count">the start capacity of the item list</param>
        public ItemWatchlist(int count)
        {
            m_EntryList = new List<ItemInfo>(count);
            m_Index = 0;
        }

        /// <summary>
        /// Initializes the Watchlist
        /// </summary>
        public void Init() => Clear();

        /// <summary>
        /// Clears items in the watch list
        /// </summary>
        public void Clear()
        {
            m_EntryList.Clear();
            m_Index = 0;
        }

        /// <summary>
        /// Add item to the watch list
        /// </summary>
        /// <param name="itemInfo">the item info to be added</param>
        public void Add(ItemInfo itemInfo) => m_EntryList.Add(itemInfo);

        /// <summary>
        /// Get next item in the list, moving index to the beginning 
        /// if last item is returned
        /// </summary>
        /// <returns>next item in the list, null if list is empty</returns>
        public ItemInfo? GetNext()
        {
            if (m_EntryList.Count == 0)
                return null;
            ItemInfo entry = m_EntryList[m_Index];
            if (++m_Index == m_EntryList.Count)
                m_Index = 0;
            return entry;
        }

        /// <summary>
        /// Get first item in the list
        /// </summary>
        /// <returns>first item in the list, null if list is empty</returns>
        public ItemInfo? GetFront() => m_EntryList.Count == 0 ? null : m_EntryList[0];

        /// <summary>
        /// Removes first item in the list and returns it
        /// </summary>
        /// <returns>first item in the list that is removed, null if list is empty</returns>
        public ItemInfo? RemoveFront()
        {
            if (m_EntryList.Count == 0)
                return null;
            ItemInfo entry = m_EntryList[0];
            m_EntryList.RemoveAt(0);
            return entry;
        }

        /// <summary>
        /// Number of items in the watch list
        /// </summary>
        /// <returns>Number of items in the watch list</returns>
        public int Count() => m_EntryList.Count;
    }
}
