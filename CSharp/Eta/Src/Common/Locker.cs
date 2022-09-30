/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

namespace Refinitiv.Eta.Common
{
    /// <summary>
    /// The base class for implementation of locking mechanism.
    /// </summary>
    abstract public class Locker
    {
        /// <summary>
        /// Holds the actual locker.
        /// </summary>
        protected ReaderWriterLockSlim _slimLock = null;

        /// <summary>
        /// Enter the critical section.
        /// </summary>
        abstract public void Enter();

        /// <summary>
        /// Leave the critical section.
        /// </summary>
        abstract public void Exit();

        /// <summary>
        /// Check wheter a thread has access to the critical section.
        /// </summary>
        abstract public bool Locked { get; }
    }

    /// <summary>
    /// Allows 1 or more Readers into C-S; no Writers.
    /// </summary>
    public class ReadLocker : Locker
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="slimLock">The <see cref="ReaderWriterLockSlim"/> for locking critical section.</param>
        public ReadLocker(ReaderWriterLockSlim slimLock)
        {
            _slimLock = slimLock;
        }

        /// <summary>
        /// Enter the critical section.
        /// </summary>
        public override void Enter()
        {
            _slimLock.EnterReadLock();
        }

        /// <summary>
        /// Leave the critical section.
        /// </summary>
        public override void Exit()
        {
            _slimLock.ExitReadLock();
        }

        /// <summary>
        /// Check wheter a thread has access to the critical section.
        /// </summary>
        public override bool Locked => _slimLock.IsReadLockHeld;
    }

    /// <summary>
    /// Allows at most 1 Writer into C-S; no Readers.
    /// </summary>
    public class WriteLocker : Locker
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="slimLock">The <see cref="ReaderWriterLockSlim"/> for locking critical section.</param>
        public WriteLocker(ReaderWriterLockSlim slimLock)
        {
            _slimLock = slimLock;
        }

        /// <summary>
        /// Enter the critical section.
        /// </summary>
        public override void Enter()
        {
            _slimLock.EnterWriteLock();
        }

        /// <summary>
        /// Leave the critical section.
        /// </summary>
        public override void Exit()
        {
            _slimLock.ExitWriteLock();
        }

        /// <summary>
        /// Check wheter a thread has access to the critical section.
        /// </summary>
        public override bool Locked => _slimLock.IsWriteLockHeld;
    }

    /// <summary>
    /// Allows anybody into anything.
    /// </summary>
    public class NoLocker : Locker
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public NoLocker()
        {
        }

        /// <summary>
        /// Enter the critical section.
        /// </summary>
        public override void Enter()
        {
            
        }

        /// <summary>
        /// Leave the critical section.
        /// </summary>
        public override void Exit()
        {
            
        }

        /// <summary>
        /// Check wheter a thread has access to the critical section.
        /// </summary>
        public override bool Locked => false;
    }
}

