/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */
using LSEG.Eta.ValueAdd.Common;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Watchlist Item Group ID of a group of streams
    /// </summary>
    internal sealed class WlItemGroup : VaNode
    {
        /// <summary>
        /// Gets or set <see cref="WlService"/> for this item group.
        /// </summary>
        public WlService? WlService { get; set; }

        /// <summary>
        /// Gets or sets a group ID.
        /// </summary>
        public Buffer? GroupId { get; set; }

        /// <summary>
        /// Gets the open stream list for this item group
        /// </summary>
        public LinkedList<WlItemStream> OpenStreamList { get; private set; } = new ();

        /// <summary>
        /// Gets a dictionary of watchlist item streams by stream Id.
        /// </summary>
        public Dictionary<int, WlItemStream> StreamIdToItemGroupDict { get; private set; } = new();

        public WlItemGroup()
        {
        }

        public void Clear()
        {
            WlService = null;
            GroupId = null;
            OpenStreamList.Clear();
            StreamIdToItemGroupDict.Clear();
        }

        public override void ReturnToPool()
        {
            Clear();

            base.ReturnToPool();
        }
    }
}
