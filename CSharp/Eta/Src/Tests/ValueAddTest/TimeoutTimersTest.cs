/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using Xunit;
using Xunit.Abstractions;
using Xunit.Categories;

using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Eta.ValuedAdd.Tests;

/// <summary>
/// Test <see cref="WlTimeoutTimer"/> and <see cref="WlTimeoutTimerManager"/>
/// </summary>
public class TimeoutTimersTest
{

    /// Capture output from tests
    private readonly ITestOutputHelper output;

    public TimeoutTimersTest(ITestOutputHelper output)
    {
        this.output = output;
    }


    #region Sequential tests

    [Fact]
    [Category("Unit")]
    [Category("Timers")]
    public void SeqTimeoutOnce_Test()
    {
        int expiredCount = 0;
        TimeSpan delay = TimeSpan.FromMilliseconds(100);
        WlTimeoutTimerManager manager = new ();
        WlTimeoutTimerGroup timerGroup = manager.CreateTimerGroup((long)delay.TotalMilliseconds);
        WlTimeoutTimer timer = new (timerGroup, (t) =>
        {
            output.WriteLine("Timed out callback");
            ++expiredCount;
        });
        timer.Start();
        manager.DetectExpiredTimers(out var firstTimeoutTs);

        long firstTimeoutDelay = firstTimeoutTs - ReactorUtil.GetCurrentTimeMilliSecond();
        // we don't expect a timer to expire immediately after it was started
        Assert.Equal(0, expiredCount);
        Assert.True(firstTimeoutDelay > 0);
        Assert.True(firstTimeoutDelay <= delay.TotalMilliseconds,
            $"firstTimeout: {firstTimeoutDelay} should be <= than delay: {delay.TotalMilliseconds}");
        Thread.Sleep(TimeSpan.FromMilliseconds(firstTimeoutDelay) + TimeSpan.FromMilliseconds(5));

        Assert.True(manager.DetectExpiredTimers(out var beforeReactorTimeout));

        // as there are no timers left, but without housekeeping done on the reactor side,
        // next timeout remains the same
        Assert.Equal(timer.Timeout, beforeReactorTimeout);

        // callback wasn't invoked yet
        Assert.Equal(0, expiredCount);

        // now, get the timer and invoke callback
        manager.DispatchExpiredTimers();

        // DispatchExpiredTimers deactivates expired timer, now manager has no timers to manage
        Assert.False(manager.DetectExpiredTimers(out var noTimersLeft));
        Assert.Equal(long.MaxValue, noTimersLeft);

        timer.Stop();

        Assert.Equal(1, expiredCount);
    }

    [Fact]
    [Category("Unit")]
    [Category("Timers")]
    public void SeqTimeoutGroups_Test()
    {
        // this test works with several groups
        int expiredCount = 0;
        TimeSpan delay = TimeSpan.FromMilliseconds(50);
        WlTimeoutTimerManager manager = new ();
        WlTimeoutTimerGroup timerGroup1 = manager.CreateTimerGroup((long)delay.TotalMilliseconds);
        WlTimeoutTimerGroup timerGroup2 = manager.CreateTimerGroup((long)delay.TotalMilliseconds);

        WlTimeoutTimer timer1 = new (timerGroup1, (t) =>
        {
            output.WriteLine("Timed out callback 1");
            ++expiredCount;
        });

        WlTimeoutTimer timer2 = new (timerGroup2, (t) =>
        {
            output.WriteLine("Timed out callback 2");
            ++expiredCount;
        });

        timer1.Start();
        timer2.Start();

        Assert.False(manager.DetectExpiredTimers(out var firstTimeoutTs));

        long firstTimeoutDelay = firstTimeoutTs - ReactorUtil.GetCurrentTimeMilliSecond();

        // we don't expect a timer to expire immediately after it was started
        Assert.Equal(0, expiredCount);
        Thread.Sleep(TimeSpan.FromMilliseconds(firstTimeoutDelay + 15));

        // now is the time to expect timed out timer, the reactor side of the manager
        // (DispatchExpiredTimers) wasn't invoked yet, and therefore the timeout timestamp
        // remains the same
        Assert.True(manager.DetectExpiredTimers(out var secondTimeoutTs));

        Assert.Equal(firstTimeoutTs, secondTimeoutTs);

        // callback wasn't invoked yet
        Assert.Equal(0, expiredCount);

        // now both timers are timed out, dispatch them
        manager.DispatchExpiredTimers();

        timer1.Stop();
        timer2.Stop();

        Assert.Equal(2, expiredCount);
    }

    [Fact]
    [Category("Unit")]
    [Category("Timers")]
    public void SeqTimeoutDifferentGroups_Test()
    {
        // this test works with two groups, with one timer per group, second group defines
        // timeout to be too long for the timer to expire during test
        int expiredCount = 0;
        TimeSpan delay = TimeSpan.FromMilliseconds(50);
        WlTimeoutTimerManager manager = new ();
        WlTimeoutTimerGroup timerGroup1 = manager.CreateTimerGroup((long)delay.TotalMilliseconds);
        WlTimeoutTimerGroup timerGroup2 = manager.CreateTimerGroup(10 * (long)delay.TotalMilliseconds);

        WlTimeoutTimer timer1 = new (timerGroup1, (t) =>
        {
            output.WriteLine("Timed out callback 1");
            ++expiredCount;
        });

        WlTimeoutTimer timer2 = new (timerGroup2, (t) =>
        {
            output.WriteLine("Timed out callback 2");
            ++expiredCount;
        });

        timer1.Start();
        timer2.Start();

        Assert.False(manager.DetectExpiredTimers(out var firstTimeoutTs));

        long firstTimeoutDelay = firstTimeoutTs - ReactorUtil.GetCurrentTimeMilliSecond();

        // we don't expect a timer to expire immediately after it was started
        Assert.Equal(0, expiredCount);
        Assert.True(firstTimeoutDelay > 0);
        Assert.True(firstTimeoutDelay <= delay.TotalMilliseconds,
            $"firstTimeout: {firstTimeoutDelay} should be <= than delay: {delay.TotalMilliseconds}");

        Thread.Sleep(TimeSpan.FromMilliseconds(firstTimeoutDelay) + TimeSpan.FromMilliseconds(5));

        // now is the time to expect timed out timer, but one timer still remains
        Assert.True(manager.DetectExpiredTimers(out long oneTimerLeft));

        // callback wasn't invoked yet
        Assert.Equal(0, expiredCount);

        // but since there are no timers to expire, their expiration is in the very distant future
        Assert.NotEqual(long.MaxValue, oneTimerLeft);

        // now, get the timer and invoke callback
        manager.DispatchExpiredTimers();

        Assert.False(manager.DetectExpiredTimers(out var secondTimerLeft));
        Assert.True(firstTimeoutTs < secondTimerLeft);
        Assert.Equal(timer2.Timeout, secondTimerLeft);

        timer1.Stop();
        timer2.Stop();

        // only first timer is expected to timeout
        Assert.Equal(1, expiredCount);
    }

    [Fact]
    [Category("Unit")]
    [Category("Timers")]
    public void SeqDontCallbackStoppedTimer_Test()
    {
        WlTimeoutTimerManager manager = new ();
        WlTimeoutTimerGroup group = manager.CreateTimerGroup(1000);

        bool invoked = false;
        WlTimeoutTimer timer = new (group, (WlTimeoutTimer t) => invoked = true);

        timer.Start();
        timer.Stop();

        timer.InvokeCallback();

        Assert.False(invoked);

        manager.ClearTimerGroup(group);
    }


    [Fact]
    [Category("Unit")]
    [Category("Timers")]
    public void SeqCallbackActiveTimer_Test()
    {
        WlTimeoutTimerManager manager = new ();
        WlTimeoutTimerGroup group = manager.CreateTimerGroup(1000);

        bool invoked = false;
        WlTimeoutTimer timer = new (group, (WlTimeoutTimer t) => invoked = true);

        timer.Start();
        timer.InvokeCallback();

        Assert.True(invoked);

        manager.ClearTimerGroup(group);
    }

    #endregion

    #region Concurrent tests utility code

    /// <summary>
    ///
    /// </summary>
    /// <param name="testDuration">for how long the test is run</param>
    /// <param name="groupDelay">delay for timers group - how long before a timer expires</param>
    /// <param name="timersResetTimeout">how long to wait before restarting the timer</param>
    /// <param name="expectedExpirationCount">number of timers expected to time-out</param>
    private void TestRunner(TimeSpan testDuration,
                            TimeSpan groupDelay,
                            TimeSpan[] timersResetTimeout,
                            int expectedExpirationCount = 0)
    {
        long start = ReactorUtil.GetCurrentTimeMilliSecond();

        WlTimeoutTimerManager timerManager = new ();
        WlTimeoutTimerGroup timerGroup = timerManager.CreateTimerGroup((long)groupDelay.TotalMilliseconds);
        CancellationTokenSource tokenSource = new ();

        WlTimeoutTimer[] timers = new WlTimeoutTimer[timersResetTimeout.Length];
        DateTime[] resetWhen = new DateTime[timersResetTimeout.Length];

        // emulate the application/reactor thread
        Thread reactor = new (() =>
        {
            int expiredCount = 0;

            for (int i = 0; i < timers.Length; ++i)
            {
                timers[i] = new WlTimeoutTimer(timerGroup, (t) =>
                {
                    output.WriteLine($"{ReactorUtil.GetCurrentTimeMilliSecond() - start}: Reactor invoked callback");
                    ++expiredCount;
                });

                timers[i].Start();
                resetWhen[i] = DateTime.Now + timersResetTimeout[i];
            }

            output.WriteLine($"{ReactorUtil.GetCurrentTimeMilliSecond() - start}: Reactor started");

            while (!tokenSource.Token.IsCancellationRequested)
            {
                timerManager.DispatchExpiredTimers();

                for (int i = 0; i < timers.Length; ++i)
                {
                    if (timers[i].IsActive && DateTime.Now > resetWhen[i])
                    {
                        output.WriteLine($"{ReactorUtil.GetCurrentTimeMilliSecond() - start}: Reactor reset timer {i}: {timers[i].Timeout - start}");
                        timers[i].Stop();
                        timers[i].Start();
                        resetWhen[i] = DateTime.Now + timersResetTimeout[i];
                    }
                }

                Thread.Sleep(TimeSpan.FromMilliseconds(1));
            }
            output.WriteLine($"{ReactorUtil.GetCurrentTimeMilliSecond() - start}: Reactor finished");
            Assert.Equal(expectedExpirationCount, expiredCount);
        });

        // emulate the worker thread
        Thread worker = new (() =>
        {
            output.WriteLine($"{ReactorUtil.GetCurrentTimeMilliSecond() - start}: Worker started");
            while (!tokenSource.Token.IsCancellationRequested)
            {
                long nextExpirationTs;
                while (timerManager.DetectExpiredTimers(out nextExpirationTs))
                {
                    // real Worker thread would send an event over ReactorChannel to
                    // wake-up the application and handle the expired timer
                    output.WriteLine($"{ReactorUtil.GetCurrentTimeMilliSecond() - start}: Worker detected expired timer!");
                }
                Assert.True(nextExpirationTs > ReactorUtil.GetCurrentTimeMilliSecond());
                long delay = Math.Min(50, nextExpirationTs - ReactorUtil.GetCurrentTimeMilliSecond());
                Thread.Sleep(TimeSpan.FromMilliseconds(delay));
            }
            output.WriteLine($"{ReactorUtil.GetCurrentTimeMilliSecond() - start}: Worker finished");
        });

        // execute test for specified duration
        tokenSource.CancelAfter((int)testDuration.TotalMilliseconds);

        reactor.Start();
        worker.Start();

        reactor.Join();
        worker.Join();
    }

    #endregion
    #region Concurrent tests

    [Fact]
    [Category("Unit")]
    [Category("Timers")]
    public void SimpleNoTimeout_Test()
    {
        TestRunner(testDuration: TimeSpan.FromMilliseconds(100),
                   groupDelay: TimeSpan.FromMilliseconds(50),
                   timersResetTimeout: new TimeSpan[] {
                       TimeSpan.FromMilliseconds(10)
                   });
    }

    [Fact]
    [Category("Unit")]
    [Category("Timers")]
    public void SimpleTimeoutOnce_Test()
    {
        TestRunner(testDuration: TimeSpan.FromMilliseconds(100),
                   groupDelay: TimeSpan.FromMilliseconds(50),
                   timersResetTimeout: new TimeSpan[] { TimeSpan.FromMilliseconds(70) },
                   expectedExpirationCount: 1);
    }

    [Fact]
    [Category("Unit")]
    [Category("Timers")]
    public void SimpleTimeoutAll_Test()
    {
        TestRunner(testDuration: TimeSpan.FromMilliseconds(100),
                   groupDelay: TimeSpan.FromMilliseconds(50),
                   // periods between timer resets
                   timersResetTimeout: new TimeSpan[] {
                       TimeSpan.FromMilliseconds(60),
                       TimeSpan.FromMilliseconds(70),
                       TimeSpan.FromMilliseconds(80)
                   },
                   expectedExpirationCount: 3);
    }

    [Fact]
    [Category("Unit")]
    [Category("Timers")]
    public void SimpleTimeoutSome_Test()
    {
        TestRunner(testDuration: TimeSpan.FromMilliseconds(100),
                   groupDelay: TimeSpan.FromMilliseconds(50),
                   timersResetTimeout: new TimeSpan[] {
                       TimeSpan.FromMilliseconds(30),
                       TimeSpan.FromMilliseconds(80),
                       TimeSpan.FromMilliseconds(90)
                   },
                   expectedExpirationCount: 2);
    }

    #endregion
}
