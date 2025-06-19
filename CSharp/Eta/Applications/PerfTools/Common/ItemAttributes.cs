/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// Class containing the attributes that uniquely identify an item.
    /// </summary>
    public class ItemAttributes
    {
        /// <summary>
        /// Key for this item
        /// </summary>
        public IMsgKey? MsgKey { get; set; }

        /// <summary>
        /// Domain of this item
        /// </summary>
        public int DomainType { get; set; }

        public void Clear()
        {
            MsgKey?.Clear();
            DomainType = 0;
        }
    }
}
