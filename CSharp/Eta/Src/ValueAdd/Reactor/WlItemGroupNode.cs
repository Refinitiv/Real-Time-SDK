/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class WlItemGroupNode
    {
        internal class WlItemGroupNodeLink : VaDoubleLinkList<WlItemGroupNode>.ILink<WlItemGroupNode>
        {
            public WlItemGroupNode? GetPrev(WlItemGroupNode thisPrev) { return thisPrev._prev; }
            public void SetPrev(WlItemGroupNode? thisPrev, WlItemGroupNode? thatPrev) { thisPrev!._prev = thatPrev; }
            public WlItemGroupNode? GetNext(WlItemGroupNode thisNext) { return thisNext._next; }
            public void SetNext(WlItemGroupNode? thisNext, WlItemGroupNode? thatNext) { thisNext!._next = thatNext; }
        }

        internal static readonly WlItemGroupNodeLink ITEM_GROUP_NODE_LINK = new();

        internal WlItemGroupNode? _prev, _next;

        internal WlItemGroupNode(int length)
        {
            itemGroup = new();
            Buffer buffer = new Buffer();
            ByteBuffer byteBuffer = new ByteBuffer(length);
            buffer.Data(byteBuffer);
            itemGroup.GroupId = buffer;
        }

        internal WlItemGroupNode() { }

        internal WlItemGroup? itemGroup;
    }
}
