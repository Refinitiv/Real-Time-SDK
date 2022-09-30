/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Common
{
    /// <summary>
    /// A thread-safe version of the Value Add FIFO queue.
    /// </summary>
    public class VaConcurrentQueue : VaQueue
    {
        ReaderWriterLockSlim _lock = new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);

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