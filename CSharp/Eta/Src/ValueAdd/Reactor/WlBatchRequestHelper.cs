/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// This class is used to merge WlItemRequest with the same item name in a batch request.
    /// </summary>
    internal class WlBatchRequestHelper
    {
        public IDictionary<int, WlItemRequest>? WlRequestDict { get; set; }
        public IDictionary<int, IRequestMsg>? RequestMsgDict { get; set; }

        public int CurrentStreamId { get; set; }

        public List<string>? ItemNames { get; set; }

        /// <summary>
        /// Keeps additional WLItemRequest objects with the same request attributes
        /// </summary>
        public List<WlItemRequest> WlItemRequestList { get; set; } = new ();

        /// <summary>
        /// Used to keep track of getting the WlItemRequest from the <see cref="WlItemRequestList"/>
        /// </summary>
        public int ItemRequestIndex { get; set; } = -1;
    }
}
