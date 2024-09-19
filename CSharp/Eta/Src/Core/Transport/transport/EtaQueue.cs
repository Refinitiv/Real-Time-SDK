/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;

namespace LSEG.Eta.Transports
{
    internal class EtaQueue
    {
        internal EtaNode _head;
        internal EtaNode _tail;

        private static bool DEBUG = false; // for turning on debug

        public EtaQueue()
        {
            _tail = null;
        }

        public void Add(EtaNode node)
        {
            if (DEBUG)
                VerifyQueue();

            if (node == null)
                return;

            Debug.Assert(node.Next == null);

            if (_tail == null)
            {
                Debug.Assert(_head == null, "EtaQueue.add(): unexpectedly found _tail null but head was not null!");
                Debug.Assert(Size == 0);
                // Queue is empty, simply add node.
                _head = node;
            }
            else
            {
                // Add node to next of the current tail
                _tail.Next = node;
            }

            // update tail to be the node passed in
            _tail = node;
            _tail.Next = null;
            ++Size;

            if (DEBUG)
                VerifyQueue();
        }

        public EtaNode Poll()
        {
            if (DEBUG)
                VerifyQueue();

            if (_head == null)
            {
                return null;
            }

            EtaNode node = _head;
            --Size;

            if (_head.Next == null)
            {
                Debug.Assert(Size == 0);
                // queue is empty
                _head = null;
                _tail = null;
                Size = 0;
            }
            else
            {
                // make the next node the new head
                _head = _head.Next;
            }

            node.Next = null;

            if (DEBUG)
                VerifyQueue();

            return node;
        }

        public bool Remove(EtaNode node)
        {
            if (node == null || _head == null)
                return false;

            if (_head == node)
            {
                if (_head.Next != null)
                {
                    _head = _head.Next;
                }
                else
                {
                    _head = null;
                    _tail = null;
                }
                Size--;
                node.Next = null;
                return true;
            }

            EtaNode previousNode = _head;
            EtaNode currentNode = _head.Next;
            while (currentNode != null)
            {
                if (currentNode == node)
                {
                    if (currentNode.Next != null)
                    {
                        previousNode.Next = currentNode.Next;
                    }
                    else
                    {
                        // at the tail.
                        previousNode.Next = null;
                        _tail = previousNode;
                    }
                    Size--;
                    node.Next = null;
                    return true;
                }

                // move to the next node
                previousNode = currentNode;
                currentNode = currentNode.Next;
            }
            return false;
        }

        public int Size { get; private set; }

        internal void Clear()
        {
            while (Size != 0)
            {
                Poll();
            }
        }

        internal bool Contains(EtaNode node)
        {
            bool ret = false;

            for (EtaNode qNode = _head; qNode != null; qNode = qNode.Next)
            {
                if (qNode == node)
                {
                    ret = true;
                    break;
                }
            }

            return ret;
        }

        private void VerifyQueue()
        {
            int i = 0;
            Debug.Assert(Size >= 0);
            for (EtaNode node = _head; node != null; node = node.Next)
            {
                ++i;
                if (node.Next == null)
                    Debug.Assert(node == _tail);
            }

            Debug.Assert(i == Size);
        }
    }
}
