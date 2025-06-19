/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics.CodeAnalysis;

namespace LSEG.Eta.ValueAdd.Reactor;

/// <summary>
/// Manage timeout timers groups.
/// This is the main entry point for dealing with timeout timers.
/// </summary>
///
/// <remarks>
/// <para>
/// This is an utility class to simplify work with timer groups.</para>
///
/// <para>
/// Worker thread periodically invokes <see cref="DetectExpiredTimers"/> method. This method returns
/// <c>true</c> when there is a new expired timer.</para>
///
/// <para>
/// When there is a new expired timer Worker thread sends a byte over the event socket to
/// wake-up the application thread and let it invoke the Reactor dispatch method.</para>
///
/// <para>
/// Reactor dispatch, running in the application thread, invokes
/// <see cref="DispatchExpiredTimers()"/> which
/// invokes callback method delegate of that timer instance.</para>
///
/// <para>
/// This class also implements a factory method to obtain new timer groups:
/// <see cref="CreateTimerGroup(long)"/>. This method creates a new timer group instance with
/// the specified timeout delay. For instance, one such group can be created to handle
/// request timeout for WlStream instances.</para>
///
/// </remarks>
/// <seealso cref="WlTimeoutTimerGroup"/>
internal class WlTimeoutTimerManager
{
    private readonly List<WlTimeoutTimerGroup> m_TimerGroups = new ();

    private long m_NextTimeout = long.MaxValue;

    /// <summary>
    /// Creates a timer group for the specified timeout delay.
    /// </summary>
    /// <param name="delay">timeout delay for timers in created group, in milliseconds
    /// </param>
    /// <returns>new timers group with the specified timeout delay</returns>
    internal WlTimeoutTimerGroup CreateTimerGroup(long delay)
    {
        var group = new WlTimeoutTimerGroup(this, delay);
        m_TimerGroups.Add(group);
        return group;
    }

    /// <summary>
    /// Check if there are expired timers in any of the timer groups.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This method is expected to be invoked from the <see cref="ReactorWorker"/> thread.</para>
    ///
    /// <para>
    /// It also expects Application thread to update the timestamp of the nearest timer
    /// timeout when a timer is started or stopped.</para>
    ///
    /// </remarks>
    /// <param name="nextTimerMs">timestamp in milliseconds of the next timer expected to expire</param>
    /// <returns><c>true</c> when there is an expired timer</returns>
    ///
    /// <seealso cref="UpdateNextTimeout(WlTimeoutTimer)"/>
    /// <seealso cref="OnTimerStopped(WlTimeoutTimer)"/>
    internal bool DetectExpiredTimers(out long nextTimerMs)
    {
        nextTimerMs = Interlocked.Read(ref m_NextTimeout);
        return nextTimerMs <= ReactorUtil.GetCurrentTimeMilliSecond();
    }

    /// <summary>
    /// Find expired timers and make them invoke callback methods.
    /// </summary>
    /// <remarks>
    /// Expected to be invoked from the Application thread (where Reactor code is
    /// normally executed) upon dispatching <see cref="ReactorEventImpl.ImplType.WATCHLIST_TIMEOUT"/>
    /// worker thread event.
    /// </remarks>
    internal void DispatchExpiredTimers()
    {
        long now = ReactorUtil.GetCurrentTimeMilliSecond();

        foreach (var timerGroup in m_TimerGroups)
        {
            while (timerGroup.TryGetExpiredTimer(out var expiredTimer, now))
            {
                expiredTimer.InvokeCallback();
            }
        }
    }

    internal void ClearTimerGroup(WlTimeoutTimerGroup? timerGroup)
    {
        if (timerGroup != null)
        {
            m_TimerGroups.Remove(timerGroup);
            timerGroup.Clear();
        }
    }

    #region Next Timeout management

    internal void UpdateNextTimeout(WlTimeoutTimer timer)
    {
        // When timer is started:
        // if timer timeouts before m_NextTimeout, then m_NextTimeout is set to its value
        if (timer.Timeout < Interlocked.Read(ref m_NextTimeout))
        {
            Interlocked.Exchange(ref m_NextTimeout, timer.Timeout);
        }
        // if timer timeouts after, then the m_NextTimeout is not changed
    }

    internal void OnTimerStopped(WlTimeoutTimer timer)
    {
        // When timer is stopped:
        // if stopped timer timeouts after m_NextTimeout, then do nothing
        // if stopped timer timeouts before m_NextTimeout, then the next earliest timeout
        // timer must be found and its timeout value used as m_NextTimeout
        if (timer.Timeout <= Interlocked.Read(ref m_NextTimeout))
        {
            long nextTimeout = long.MaxValue;
            foreach (var timerGroup in m_TimerGroups)
            {
                long nextTimeoutInGroup = timerGroup.GetNextTimeout();
                if (nextTimeoutInGroup < nextTimeout)
                {
                    nextTimeout = nextTimeoutInGroup;
                }
            }
            Interlocked.Exchange(ref m_NextTimeout, nextTimeout);
        }
    }

    #endregion
}
