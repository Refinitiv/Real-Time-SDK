/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Runtime.CompilerServices;

namespace LSEG.Eta.ValueAdd.Common
{
    /// <summary>
    /// A thread-safe version of the Value Add FIFO queue.
    /// </summary>
    public class VaConcurrentQueue : VaQueue
    {
        object _lockObj = new object();

        /// <summary>
        /// Adds <see cref="VaNode"/> to this queue.
        /// </summary>
        /// <param name="node">The node to add</param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override void Add(VaNode node)
        {
            lock (_lockObj)
            {
                base.Add(node);
            }
        }

        /// <summary>
        /// Polls a <see cref="VaNode"/> from this queue.
        /// </summary>
        /// <returns>Removes the oldest <see cref="VaNode"/></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override VaNode? Poll()
        {
            lock (_lockObj)
            {
                return base.Poll();
            }
        }

        /// <summary>
        /// Returns but does not remove the head of the queue.
        /// </summary>
        /// <returns>The head of the queue</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override VaNode? Peek()
        {
            lock (_lockObj)
            {
                return base.Peek();
            }
        }

        /// <summary>
        /// Removes a node from the queue.
        /// </summary>
        /// <param name="node">The node to remove</param>
        /// <returns><c>true</c> if the node was in the queue, or <c>false</c> if the node wasn't</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override bool Remove(VaNode node)
        {
            lock (_lockObj)
            {
                return base.Remove(node);
            }
        }

        /// <summary>
        /// Returns the size of the queue.
        /// </summary>
        /// <returns>The size of queue</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override int Size()
        {
            lock ( _lockObj)
            {
                return base.Size();
            }
        }
    }
}