/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Common
{
    /// <summary>
    /// A thread-safe version of the Value Add FIFO queue.
    /// </summary>
    public class VaConcurrentQueue : VaQueue
    {
        ReaderWriterLockSlim _lock = new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);

        /// <summary>
        /// Adds <see cref="VaNode"/> to this queue.
        /// </summary>
        /// <param name="node">The node to add</param>
        public override void Add(VaNode node)
        {
            _lock.EnterWriteLock();
            try
            {
                base.Add(node);
            }
            finally 
            { 
                _lock.ExitWriteLock();
            }
        }

        /// <summary>
        /// Polls a <see cref="VaNode"/> from this queue.
        /// </summary>
        /// <returns>Removes the oldest <see cref="VaNode"/></returns>
        public override VaNode? Poll()
        {
            _lock.EnterWriteLock();
            try
            {
                return base.Poll();
            }
            finally
            {
                _lock.ExitWriteLock();
            }
        }

        /// <summary>
        /// Returns but does not remove the head of the queue.
        /// </summary>
        /// <returns>The head of the queue</returns>
        public override VaNode? Peek()
        {
            _lock.EnterWriteLock();
            try
            {

                return base.Peek();
            }
            finally
            {
                _lock.ExitWriteLock();
            }
        }

        /// <summary>
        /// Removes a node from the queue.
        /// </summary>
        /// <param name="node">The node to remove</param>
        /// <returns><c>true</c> if the node was in the queue, or <c>false</c> if the node wasn't</returns>
        public override bool Remove(VaNode node)
        {
            _lock.EnterWriteLock();
            try
            {

                return base.Remove(node);
            }
            finally
            {
                _lock.ExitWriteLock();
            }
        }

        /// <summary>
        /// Returns the size of the queue.
        /// </summary>
        /// <returns>The size of queue</returns>
        public override int Size()
        {
            _lock.EnterWriteLock();
            try
            {
                return base.Size();
            }
            finally
            {
                _lock.ExitWriteLock();
            }
        }
    }
}