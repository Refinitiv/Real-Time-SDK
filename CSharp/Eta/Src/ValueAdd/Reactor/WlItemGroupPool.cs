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
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class WlItemGroupPool
    {
        VaDoubleLinkList<WlItemGroupNode>[] itemGroupListArray;
        VaDoubleLinkList<WlItemGroupNode>[] emptyItemGroupListArray;
        int minLengthPow;

        internal WlItemGroupPool(int[] listSize, int numOfLengths = 10, int minLengthPow = 2)
        {
            Debug.Assert(listSize.Length >= numOfLengths);

            this.minLengthPow = minLengthPow;

            itemGroupListArray = new VaDoubleLinkList<WlItemGroupNode>[numOfLengths];
            emptyItemGroupListArray = new VaDoubleLinkList<WlItemGroupNode>[numOfLengths];
            for (int i = 0; i < numOfLengths; i++)
            {
                var itemGroupList = new VaDoubleLinkList<WlItemGroupNode>();
                var emptyItemGroupList = new VaDoubleLinkList<WlItemGroupNode>();
                int length = (int)Math.Pow(2, minLengthPow + i);
                for (int j = 0; j < listSize[i]; j++)
                {
                    WlItemGroupNode node = new WlItemGroupNode(length);
                    itemGroupList.Push(node, WlItemGroupNode.ITEM_GROUP_NODE_LINK);
                }
                itemGroupListArray[i] = itemGroupList;
                emptyItemGroupListArray[i] = emptyItemGroupList;
            }
        }

        internal WlItemGroup GetWlItemGroup(int length)
        {
            int pow;

            if (length < Math.Pow(2, minLengthPow - 1))
            {
                pow = minLengthPow;
            }
            else
            {
                pow = (int)Math.Log2(length) + 1;
            }

            int index = pow - minLengthPow;

            if ( -1 < index && index < itemGroupListArray.Length)
            {
                var itemGroupNode = itemGroupListArray[index].Pop(WlItemGroupNode.ITEM_GROUP_NODE_LINK);
                if (itemGroupNode == null)
                {
                    itemGroupNode = new WlItemGroupNode((int)Math.Pow(2, pow));
                }
                emptyItemGroupListArray[index].Push(itemGroupNode, WlItemGroupNode.ITEM_GROUP_NODE_LINK);
                return itemGroupNode.itemGroup!;
            }
            else
            {
                WlItemGroup itemGroup = new();
                Buffer buffer = new Buffer();
                ByteBuffer byteBuffer = new ByteBuffer(length);
                buffer.Data(byteBuffer);
                itemGroup.GroupId = buffer;
                return itemGroup;
            }
        }

        internal void ReturnWlItemGroup(WlItemGroup itemGroup)
        {
            var byteBuffer = itemGroup.GroupId!.Data();
            int length = byteBuffer.Contents.Length;
            int pow = (int)Math.Log2(length);

            if (pow < minLengthPow)
            {
                pow = minLengthPow;
            }

            int index = pow - minLengthPow;

            if (-1 < index && index < emptyItemGroupListArray.Length)
            {
                itemGroup.GroupId.Clear();
                byteBuffer.Clear();
                itemGroup.GroupId.Data(byteBuffer);

                var itemGroupNode = emptyItemGroupListArray[index].Pop(WlItemGroupNode.ITEM_GROUP_NODE_LINK);
                if (itemGroupNode == null)
                {
                    itemGroupNode = new WlItemGroupNode();
                }
                itemGroupNode.itemGroup = itemGroup;
                itemGroupListArray[index].PushBack(itemGroupNode, WlItemGroupNode.ITEM_GROUP_NODE_LINK);
            }
        }
    }
}
