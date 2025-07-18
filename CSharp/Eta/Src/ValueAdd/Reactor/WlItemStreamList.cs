/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class WlItemStreamList
    {
        private WlItemStream? _head, _tail;
        private WlItemStream? _iter;
        private int _count;

        public WlItemStreamList()
        {
            Clear();
        }

        /// <summary>
        /// Clears to default values.
        /// </summary>
        public void Clear()
        {
            _head = null;
            _tail = null;
            _count = 0;
        }

        /// <summary>
        /// Returns the number of elements in the list.
        /// </summary>
        /// <returns>The number of element</returns>
        public int Count() { return _count; }

        /// <summary>
        /// Adds an element to the front of the list.
        /// </summary>
        /// <param name="elem">The element to add</param>
        public void PushBack(WlItemStream elem)
        {
            if (_tail != null)
            {
                elem.m_prev = _tail;
                _tail.m_next = elem;
                _tail = elem;
            }
            else
            {
                elem.m_prev = null;
                _head = _tail = elem;
            }

            elem.m_next = null;
            ++_count;
        }

        /// <summary>
        /// Adds an element to the back of the list
        /// </summary>
        /// <param name="elem">the element to add</param>
        public void Push(WlItemStream elem)
        {
            if (_head != null)
            {
                elem.m_next = _head;
                _head.m_prev = elem;
                _head = elem;
            }
            else
            {
                elem.m_next = null;
                _head = _tail = elem;
            }

            elem.m_prev = null;
            ++_count;
        }

        /// <summary>
        /// Returns the first elem in the list.
        /// </summary>
        /// <returns>The first element</returns>
        public WlItemStream? Peek()
        {
            return _head;
        }

        /// <summary>
        ///  Returns the last elem in the list.
        /// </summary>
        /// <returns>the last element</returns>
        public WlItemStream? PeekTail()
        {
            return _tail;
        }

        /// <summary>
        /// Removes and returns the first elem in the list.
        /// </summary>
        /// <returns>The first element</returns>
        public WlItemStream? Pop()
        {
            WlItemStream? head = _head;
            if (head != null)
                Remove(head);
            return head;
        }

        /// <summary>
        /// Starts to iterate the link
        /// </summary>
        /// <returns>The head element of the link</returns>
        public WlItemStream? Start()
        {
            if (_head != null)
                _iter = _head.m_next;
            else
                _iter = null;

            return _head;
        }

        /// <summary>
        /// Iterates through the queue.
        /// </summary>
        /// <returns>The current element before moving next</returns>
        public WlItemStream? Forth()
        {
            WlItemStream? iter;

            iter = _iter;
            if (_iter != null)
                _iter = _iter.m_next;
            return iter;
        }

        /// <summary>
        /// Removes an element from the list.
        /// </summary>
        /// <param name="elem">the element to remove</param>
        public void Remove(WlItemStream elem)
        {
            if (_iter != null && ReferenceEquals(elem, _iter))
                _iter = elem.m_next;

            if (elem != _head && elem.m_prev == null && elem.m_next == null) // element is not in the list
                return;

            if (elem.m_prev != null)
                elem.m_prev.m_next = elem.m_next;
            else
            {
                Debug.Assert(ReferenceEquals(_head, elem));
                _head = elem.m_next;
            }

            if (elem.m_next != null)
                elem.m_next.m_prev = elem.m_prev;
            else
            {
                Debug.Assert(ReferenceEquals(_tail, elem));
                _tail = elem.m_prev;
            }

            elem.m_prev = null;
            elem.m_next = null;
            --_count;
        }
    }
}

