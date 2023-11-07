/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;
using System.Threading.Tasks;

using Xunit;
using Xunit.Categories;

using LSEG.Eta.Common;

namespace LSEG.Eta.Transports.Tests
{
    public class LockerTests
    {
        [Fact]
        [Category("Unit")]
        public void ReadLockerAllowsMultipleConcurrentReaders()
        {
            long readerCount = 0;
            long expectedCount = 13;

            var slimLock = new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);
            Locker locker = new ReadLocker(slimLock);

            Action readerAction = new Action(() =>
            {
                try
                {
                    locker.Enter();

                    Interlocked.Increment(ref readerCount);

                    Thread.Sleep(127);
                }
                finally
                {
                    locker.Exit();
                }
            });

            try
            {
                locker.Enter();

                // Create and Start readerTasks.
                Task[] readers = new Task[expectedCount];
                for (int i = 0; i < expectedCount; i++)
                {
                    readers[i] = new Task(readerAction);
                    readers[i].Start();
                }
                Task.WaitAll(readers);
            }
            finally
            {
                locker.Exit();
            }

            Assert.Equal(expectedCount, readerCount);
        }

        [Fact]
        [Category("Unit")]
        public void WriteLockerAllowsNoConcurrentWriter()
        {
            long writerCount = 0;

            var slimLock = new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);
            Locker locker = new SlimWriteLocker(slimLock);

            Action writerAction = new Action(() =>
            {
                try
                {
                    Thread.Sleep(1); // Yield our slice.

                    locker.Enter();

                    long count = Interlocked.Increment(ref writerCount);
                    Assert.Equal(1, count);

                    Thread.Sleep(250);
                }
                finally
                {
                    Interlocked.Decrement(ref writerCount);
                    locker.Exit();
                }
            });


            long taskCount = 12;

            // Create Writer Tasks.
            Task[] writers = new Task[taskCount];
            try
            {
                locker.Enter();
                Interlocked.Increment(ref writerCount);
                for (int i = 0; i < taskCount; i++)
                {
                    writers[i] = new Task(writerAction);
                    writers[i].Start();
                }
            }
            finally
            {
                Interlocked.Decrement(ref writerCount);
                locker.Exit();
            }

            Task.WaitAll(writers);

            Assert.Equal(0, writerCount);
        }

        [Fact, Category("Unit")]
        public void WriteLockerAllowsNoConcurrentReader()
        {
            long criticalCount = 0;
            long readerCount = 0;
            long taskCount = 12;

            var slimLock = new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);

            Locker writeLocker = new SlimWriteLocker(slimLock);
            Locker readLocker = new ReadLocker(slimLock);

            Action readerAction = new Action(() =>
            {
                try
                {
                    readLocker.Enter();
                    Interlocked.Increment(ref readerCount);

                    long count = Interlocked.Read(ref criticalCount);
                    Assert.Equal(0, count);

                    Thread.Sleep(100);
                }
                finally
                {
                    readLocker.Exit();
                }
            });

            Task[] readers = new Task[taskCount];
            try
            {
                writeLocker.Enter();

                Interlocked.Increment(ref criticalCount);

                //-------------------------------------------------
                // Spawn Reader Tasks while inside of Writer.
                for (int i = 0; i < taskCount; i++)
                {
                    readers[i] = new Task(readerAction);
                    readers[i].Start();
                }

                // Yield our slice so Readers have chance to run.
                Thread.Sleep(1);
            }
            finally
            {
                Interlocked.Decrement(ref criticalCount);
                writeLocker.Exit();
            }

            Task.WaitAll(readers);
            Assert.Equal(taskCount, readerCount);

            Assert.Equal(0, criticalCount);
        }
    }
}
