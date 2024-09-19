/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// Contains information about a particular item being published
    /// </summary>
    public class ItemInfo
    {
        /// <summary>
        /// Item's Stream ID
        /// </summary>
        public int StreamId { get; set; }

        /// <summary>
        /// Holds information about the item's data. This data will be different depending on the domain of the item
        /// </summary>
        public object? ItemData { get; set; }

        /// <summary>
        /// Item flags (<see cref="ItemFlags"/> enumeration)
        /// </summary>
        public int ItemFlags { get; set; }

        /// <summary>
        /// Attributes that uniquely identify this item
        /// </summary>
        public ItemAttributes Attributes { get; set; } = new ItemAttributes();

        /// <summary>
        /// Clears the item information
        /// </summary>
        public void Clear()
        {
            StreamId = 0;
            ItemData = null;
            ItemFlags = 0;
            Attributes.Clear();
        }
    }
}
