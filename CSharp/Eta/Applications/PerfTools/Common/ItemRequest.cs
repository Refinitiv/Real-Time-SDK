/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;

namespace Refinitiv.Eta.PerfTools.Common
{
    /// <summary>
    /// Item request information
    /// </summary>
    public class ItemRequest
    {
        /// <summary>
        /// Item state
        /// </summary>
        public ItemRequestState RequestState { get; set; }
        /// <summary>
        /// Storage for the item's name
        /// </summary>
        public string? ItemName { get; set; }
        /// <summary>
        /// Structure containing item information
        /// </summary>
        public ItemInfo ItemInfo { get; set; }
        /// <summary>
        ///Message key used to request this item
        /// </summary>
        public IMsgKey MsgKey { get; set; }

        public ItemRequest()
        {
            MsgKey = new MsgKey();
            ItemInfo = new ItemInfo();
        }
        public void Clear()
        {
            MsgKey.Clear();
            ItemInfo.Clear();
            RequestState = ItemRequestState.NOT_REQUESTED;
            ItemInfo.Attributes.MsgKey = MsgKey;
        }
    }
}
