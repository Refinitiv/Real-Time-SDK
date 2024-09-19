/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */
using LSEG.Eta.ValueAdd.Common;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
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
        public int WlServiceId { get; set; }

        /// <summary>
        /// Gets or sets a group ID.
        /// </summary>
        public Buffer? GroupId { get; set; }

        /// <summary>
        /// Gets the open stream list for this item group
        /// </summary>
        public WlStreamIdList OpenStreamIdDlList { get; private set; } = new();

        public WlItemGroup()
        {
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Clear()
        {
            WlServiceId = 0;
            GroupId = null;
            while (OpenStreamIdDlList.Pop() != null) { }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override void ReturnToPool()
        {
            Clear();

            base.ReturnToPool();
        }
    }
}
