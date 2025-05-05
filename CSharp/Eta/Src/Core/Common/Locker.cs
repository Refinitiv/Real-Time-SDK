/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;


namespace LSEG.Eta.Common
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
    sealed public class ReadLocker : Locker
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
    sealed public class MonitorWriteLocker : Locker
    {
        private object _lock;

        /// <summary>
        /// Constructor for monitor Locker
        /// </summary>
        /// <param name="lockObject">the object which monitor is captured</param>
        public MonitorWriteLocker(object lockObject)
        {
            _lock = lockObject;
        }

        /// <summary>
        /// Enter the critical section.
        /// </summary>
        public override void Enter()
        {
            Monitor.Enter(_lock);
        }

        /// <summary>
        /// Leave the critical section.
        /// </summary>
        public override void Exit()
        {
            Monitor.Exit(_lock);
        }

        /// <summary>
        /// Check wheter a thread has access to the critical section.
        /// </summary>
        public override bool Locked => Monitor.IsEntered(_lock);
    }

    /// <summary>
    /// Locker class
    /// </summary>
    sealed public class SlimWriteLocker : Locker
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="slimLock">The <see cref="ReaderWriterLockSlim"/> for locking critical section.</param>
        public SlimWriteLocker(ReaderWriterLockSlim slimLock)
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
    sealed public class NoLocker : Locker
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

    /// <summary>
    /// The class for implementation of locking mechanism via RAII.
    /// <see cref="EnterLockScope(Locker, bool)"/> method performs locking and returns disposable object that will release lock on its <c>Dispose</c>
    /// method call. With <c>using</c> statement this guarantees unlocking even in case of exception without <c>try/finally</c> construct.
    /// Note that returned objects allocated on stack, so there is no additional heap allocations performed.
    /// </summary>
    public static class LockerExtensions
    {
        /// <summary>
        /// Acquires lock on <see cref="Locker"/>.
        /// </summary>
        /// <param name="locker">Lock object to be acquired.</param>
        /// <param name="noLockIfAlreadyLocked">If <c>true</c> is passed and <paramref name="locker"/> already locked then do nothing.</param>
        /// <returns>Returns disposable object that releases read lock on <see cref="LockerScope.Dispose"/> method call.</returns>
        public static LockerScope EnterLockScope(this Locker locker, bool noLockIfAlreadyLocked = true) => new LockerScope(locker, noLockIfAlreadyLocked);

        /// <inheritdoc/>
        public ref struct LockerScope
        {
            private readonly Locker m_Locker;
            private readonly bool m_ShouldUnlock;

            /// <inheritdoc/>
            public LockerScope(Locker locker, bool noLockIfAlreadyLocked)
            {
                m_Locker = locker;
                if (noLockIfAlreadyLocked && !m_Locker.Locked || !noLockIfAlreadyLocked)
                {
                    m_Locker.Enter();
                    m_ShouldUnlock = true;
                }
                else
                {
                    m_ShouldUnlock = false;
                }
            }

            /// <inheritdoc/>
            public void Dispose()
            {
                if (m_ShouldUnlock && m_Locker.Locked)
                {
                    m_Locker.Exit();
                }
            }
        }
    }
}

