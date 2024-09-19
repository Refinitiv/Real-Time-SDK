/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.PerfTools.Common
{
    public class ItemInfoPool
    {
        private readonly ItemInfo[] pool;
        private int current;

        public ItemInfoPool(int capacity, Func<ItemInfo> createItemInfo)
        {
            pool = new ItemInfo[capacity];
            for (int i = 0; i < capacity; i++)
            {
                pool[i] = createItemInfo();
            }

            current = capacity - 1;
        }

        public bool ReturnToPool(ItemInfo itemInfo)
        {
            if (current < pool.Length - 1)
            {
                itemInfo.Clear();
                pool[++current] = itemInfo;
                return true;
            }

            return false;
        }

        public ItemInfo? GetFromPool()
        {
            if (current >= 0)
            {
                return pool[current--];
            }
            return null;
        }
    }
}
