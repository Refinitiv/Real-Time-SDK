/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.ValueAdd.Common
{
    /// <summary>
    /// A FIFO queue.
    /// </summary>
    public class VaQueue
    {
        /// <summary>
        /// The reference to the <see cref="VaNode"/> at the head of the queue.
        /// </summary>
        protected VaNode? _head;

        /// <summary>
        /// The reference to the <see cref="VaNode"/> at the tail of the queue.
        /// </summary>
        protected VaNode? _tail;
        private int _size;

        /// <summary>
        /// Adds to the tail of the queue.
        /// </summary>
        /// <param name="node">The node to add</param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        virtual public void Add(VaNode node)
        {
            if (node is null)
                return;

            if (_tail is null)
            {
                Debug.Assert(_head is null, "VaQueue.Add(): unexpectedly found _tail null but head was not null!");
                // Queue is empty, simply add node.
                _head = node;
            }
            else
            {
                // Add node to next of the current tail.
                _tail.Next = node;
            }

            // update tail to be the node passed in.
            _tail = node;
            _tail.Next = null;
            _size++;
        }

        /// <summary>
        /// Polls a <see cref="VaNode"/> from this queue.
        /// </summary>
        /// <returns>Removes the oldest <see cref="VaNode"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        virtual public VaNode? Poll()
        {
            if (_head == null)
            {
                return null;
            }

            VaNode? node = _head;
            _size--;

            if (_head.Next == null)
            {
                // queue is empty
                _head = null;
                _tail = null;
                _size = 0;
            }
            else
            {
                // make the next node the new head
                _head = _head.Next;
            }

            return node;
        }

        /// <summary>
        /// Returns but does not remove the head of the queue.
        /// </summary>
        /// <returns>The head of the queue</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        virtual public VaNode? Peek()
        {
            return _head;
        }

        /// <summary>
        /// Removes a node from the queue.
        /// </summary>
        /// <param name="node">The node to remove</param>
        /// <returns><c>true</c> if the node was in the queue, or <c>false</c> if the node wasn't</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        virtual public bool Remove(VaNode node)
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
                _size--;
                return true;
            }

            VaNode? previousNode = _head;
            VaNode? currentNode = _head.Next;
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
                    _size--;
                    return true;
                }

                // move to the next node
                previousNode = currentNode;
                currentNode = currentNode.Next;
            }

            return false;
        }

        /// <summary>
        /// Returns the size of the queue.
        /// </summary>
        /// <returns>The size of the queue</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        virtual public int Size()
        {
            return _size;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void VerifyQueue()
        {
            int i = 0;
            Debug.Assert(_size >= 0);
            for (VaNode? node = _head; node != null; node = node.Next)
            {
                ++i;
                if (node.Next == null)
                    Debug.Assert(node == _tail);
            }

            Debug.Assert(i == _size);
        }
    }
}
