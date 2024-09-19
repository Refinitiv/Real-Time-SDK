/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace LSEG.Eta.ValueAdd.Common
{
	/// <summary>
	/// Double-linked List class for maintaining objects that might have multiple links, so that
	/// those objects can be in multiple lists simultaneously. The object does this by implementing
	/// instances of VaDoubleLinkList.Link. When using list functions, pass the appropriate instance
	/// of VaDoubleLinkList.Link as a parameter. This method also allows those objects to have such
	/// links without inheriting.
	/// </summary>
	/// <typeparam name="T">The element type</typeparam>
	public class VaDoubleLinkList<T> where T : class
    {
		private T? _head, _tail;
		private T? _iter;
		private int _count;
		private static readonly bool DEBUG_LIST = false;

		/// <summary>
		/// Used by objects to implement one or more link accessors. When calling list functions,
		/// provide an instance of the appropriate Link implementation, which tells it which links
		/// to use for browsing and editing the list.
		/// </summary>
		/// <typeparam name="V">The element type</typeparam>
		public interface ILink<V>
		{
			/// <summary>
			/// Gets the given element's previous link
			/// </summary>
			/// <param name="thisPrev">The element to get previous link</param>
			/// <returns>The element's previous link</returns>
			V? GetPrev(V thisPrev);

			/// <summary>
			/// Sets the given element's previous link
			/// </summary>
			/// <param name="thisPrev">The element to set previous link</param>
			/// <param name="thatPrev">The element's previous link</param>
			void SetPrev(V? thisPrev, V? thatPrev);

			/// <summary>
			/// Gets the given element's next link 
			/// </summary>
			/// <param name="thisNext">The element to get next link</param>
			/// <returns>The element's next link</returns>
			V? GetNext(V thisNext);

			/// <summary>
			/// Sets the given element's next link
			/// </summary>
			/// <param name="thisNext">The element to set next link</param>
			/// <param name="thatNext">The element's next link</param>
			void SetNext(V? thisNext, V? thatNext);
		}

		/// <summary>
		/// The constuctor.
		/// </summary>
		public VaDoubleLinkList()
		{
			Clear();
        }

        /// <summary>
        /// Clears to default values.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int Count() { return _count; }

        /// <summary>
        /// Adds an element to the front of the list.
        /// </summary>
        /// <param name="elem">The element to add</param>
        /// <param name="queueLink">The link for the element</param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void PushBack(T elem, ILink<T> queueLink)
		{
			if (DEBUG_LIST) VerifyList(queueLink);
			if (_tail != null)
			{
				queueLink.SetPrev(elem, _tail);
				queueLink.SetNext(_tail, elem);
				_tail = elem;
			}
			else
			{
				queueLink.SetPrev(elem, null);
				_head = _tail = elem;
			}

			queueLink.SetNext(elem, null);
			++_count;
			if (DEBUG_LIST) VerifyList(queueLink);
		}

        /// <summary>
        /// Adds an element to the back of the list
        /// </summary>
        /// <param name="elem">the element to add</param>
        /// <param name="queueLink">The link for the element</param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Push(T elem, ILink<T> queueLink)
		{
			if (DEBUG_LIST) VerifyList(queueLink);
			if (_head != null)
			{
				queueLink.SetNext(elem, _head);
				queueLink.SetPrev(_head, elem);
				_head = elem;
			}
			else
			{
				queueLink.SetNext(elem, null);
				_head = _tail = elem;
			}

			queueLink.SetPrev(elem, null);
			++_count;
			if (DEBUG_LIST) VerifyList(queueLink);
		}

        /// <summary>
        /// Adds an element before the element in the list
        /// </summary>
        /// <param name="thisElement">The element to insert before</param>
        /// <param name="newElement">The new element to be inserted to the list</param>
        /// <param name="queueLink">The link for the element</param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void InsertBefore(T thisElement, T newElement, ILink<T> queueLink)
		{
			if (DEBUG_LIST) VerifyList(queueLink);
			if (queueLink.GetPrev(thisElement) != null)
				queueLink.SetNext(queueLink.GetPrev(thisElement), newElement);
			queueLink.SetPrev(newElement, queueLink.GetPrev(thisElement));
			queueLink.SetNext(newElement, thisElement);
			queueLink.SetPrev(thisElement, newElement);

			if (ReferenceEquals(thisElement, _head))
				_head = newElement;

			++_count;
			if (DEBUG_LIST) VerifyList(queueLink);
		}

        /// <summary>
        /// Adds an element after the element in the list
        /// </summary>
        /// <param name="thisElement">The element to insert after</param>
        /// <param name="newElement">The new element to be inserted to the list</param>
        /// <param name="queueLink">The link for the element</param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void InsertAfter(T thisElement, T newElement, ILink<T> queueLink)
		{
			if (DEBUG_LIST) VerifyList(queueLink);
			if (queueLink.GetNext(thisElement) != null)
				queueLink.SetPrev(queueLink.GetNext(thisElement), newElement);
			queueLink.SetNext(newElement, queueLink.GetNext(thisElement));
			queueLink.SetPrev(newElement, thisElement);
			queueLink.SetNext(thisElement, newElement);

			if (ReferenceEquals(thisElement, _tail))
				_tail = newElement;

			++_count;
			if (DEBUG_LIST) VerifyList(queueLink);
		}

        /// <summary>
        /// Returns the first elem in the list.
        /// </summary>
        /// <returns>The first element</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public T? Peek()
		{
			return _head;
		}

        /// <summary>
        ///  Returns the last elem in the list.
        /// </summary>
        /// <returns>the last element</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public T? PeekTail()
		{
			return _tail;
		}

        /// <summary>
        /// Removes and returns the first elem in the list.
        /// </summary>
        /// <param name="queueLink">The element's link</param>
        /// <returns>The first element</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public T? Pop(ILink<T> queueLink)
		{
			T? head = _head;
			if (head != null)
				Remove(head, queueLink);
			return head;
		}

        /// <summary>
        /// Starts to iterate the link
        /// </summary>
        /// <param name="queueLink">The link for the element</param>
        /// <returns>The head element of the link</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public T? Start(ILink<T> queueLink)
		{
			if (_head != null)
				_iter = queueLink.GetNext(_head);
			else
				_iter = null;

			return _head;
		}

        /// <summary>
        /// Iterates through the queue.
        /// </summary>
        /// <param name="queueLink">The link for the element</param>
        /// <returns>The current element before moving next</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public T? Forth(ILink<T> queueLink)
		{
			T? iter;

			iter = _iter;
			if (_iter != null)
				_iter = queueLink.GetNext(_iter);
			return iter;
		}

        /// <summary>
        /// Removes an element from the list.
        /// </summary>
        /// <param name="elem">the element to remove</param>
        /// <param name="queueLink">the element's link</param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Remove(T elem, ILink<T> queueLink)
		{
			if (_iter != null && ReferenceEquals(elem,_iter))
				_iter = queueLink.GetNext(elem);

			if (elem != _head && queueLink.GetPrev(elem) == null && queueLink.GetNext(elem) == null) // element is not in the list
				return;

			if (DEBUG_LIST) VerifyList(queueLink);
			if (queueLink.GetPrev(elem) != null)
				queueLink.SetNext(queueLink.GetPrev(elem), queueLink.GetNext(elem));
			else
			{
				Debug.Assert(ReferenceEquals(_head,elem));
				_head = queueLink.GetNext(elem);
			}

			if (queueLink.GetNext(elem) != null)
				queueLink.SetPrev(queueLink.GetNext(elem), queueLink.GetPrev(elem));
			else
			{
				Debug.Assert(ReferenceEquals(_tail,elem));
				_tail = queueLink.GetPrev(elem);
			}

			queueLink.SetNext(elem, null);
			queueLink.SetPrev(elem, null);
			--_count;

			if (DEBUG_LIST) VerifyList(queueLink);
		}

        /// <summary>
        /// Appends to another list
        /// </summary>
        /// <param name="otherList">The list to append the link to</param>
        /// <param name="queueLink">the element's link</param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Append(VaDoubleLinkList<T> otherList, ILink<T> queueLink)
		{
			if (DEBUG_LIST) VerifyList(queueLink);

			if (otherList._head == null)
				return;

			/* Link the lists. */
			if (_head != null)
			{
				queueLink.SetNext(_tail, otherList._head);
				queueLink.SetPrev(otherList._head, _tail);
				_tail = otherList._tail;
			}
			else
			{
				_head = otherList._head;
				_tail = otherList._tail;
			}

			/* Update count and clear old list. */
			_count += otherList._count;
			otherList._head = null;
			otherList._tail = null;
			otherList._count = 0;

			if (DEBUG_LIST) VerifyList(queueLink);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        void VerifyList(ILink<T> queueLink)
		{
			int i = 0;
			Debug.Assert(_count >= 0);
			for (T? link = _head; link != null; link = queueLink.GetNext(link))
			{
				++i;
				if (queueLink.GetNext(link) == null)
					Debug.Assert(ReferenceEquals(link,_tail));
			}

			Debug.Assert(i == _count);
		}

        /// <summary>
        /// Checks whether the element in the list.
        /// </summary>
        /// <param name="elem">the element</param>
        /// <param name="queueLink">the element's link</param>
        /// <returns><c>true</c> if an element is contained in the list</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public bool Contains(T elem, ILink<T> queueLink)
		{
			for (T? link = _head; link != null; link = queueLink.GetNext(link))
			{
				if (ReferenceEquals(link, elem))
					return true;
			}

			return false;
		}
	}
}
