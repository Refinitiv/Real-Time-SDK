/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Common;
using LSEG.Eta.ValueAdd.Rdm;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal sealed class WlService
    {
        public Service? RdmService { get; internal set; }
        public long NumOutstandingRequests { get; internal set; }

        /// <summary>
        /// Gets or sets the waiting request list to handle cases where request could not be submitted
        /// with directory stream up but the open windows wasn't open
        /// </summary>
        public LinkedList<WlItemRequest> WaitingRequestList { get; set; } = new ();

        public WlStreamIdList StreamIdDlList { get; set; } = new();

        /// <summary>
        /// Dictionary of item groups, by group ID.
        /// </summary>
        public Dictionary<Buffer, WlItemGroup> ItemGroupDict = new (50);
    }
}
