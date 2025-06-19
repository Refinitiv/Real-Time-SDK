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
/// Manages timeout timers with identical timeout delay.
/// </summary>
/// <remarks>
/// <para>
/// Expects to be accessed only from the application (reactor) thread.</para>
///
/// <para>
/// Application thread is expected to start-stop timers and check for timed-out
/// timers.</para>
///
/// </remarks>
internal class WlTimeoutTimerGroup
{
    /// <summary>
    /// Delay before an active timer expires in milliseconds.
    /// </summary>
    public long GroupDelay { init; get; }

    /// <summary>
    /// List of active timers for this group.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Since time-out delay is the same for all timers in a group, the timers expire in
    /// the order they were started.</para>
    ///
    /// <para>
    /// Once a timer is stopped, its entry can be removed from the list without disrupting
    /// the general order.</para>
    ///
    /// <para>
    /// When a timer is started, its entry is appended to the this list tail.</para>
    ///
    /// <para>
    /// First entry in this list (its head) is the timer with the earliest expiration
    /// time, while the last (tail) is the timer with the latest expiration time.</para>
    ///
    /// </remarks>
    private readonly LinkedList<WlTimeoutTimer> m_ActiveTimers = new();

    private readonly WlTimeoutTimerManager m_TimerManager;

    /// <summary>
    /// Constructs a new timeout timers group with the specified delay.
    /// </summary>
    /// <param name="manager">manager that manages this timer group</param>
    /// <param name="delay">timeout delay in milliseconds for individual
    ///   timers in this group</param>
    internal WlTimeoutTimerGroup(WlTimeoutTimerManager manager, long delay)
    {
        m_TimerManager = manager;
        GroupDelay = delay;
    }

    public void Clear()
    {
        foreach (var activeTimer in m_ActiveTimers)
        {
            activeTimer.Clear();
        }
        m_ActiveTimers.Clear();
    }

    #region Timekeeping and Actions on Timers

    /// <summary>
    /// Called by the worker thread to detect newly expired timers
    /// </summary>
    /// <remarks>
    /// Expired timer is removed from the active timers list and therefore is not checked
    /// upon subsequent calls.
    /// </remarks>
    ///
    /// <returns><c>true</c> if there is a timer that has just expired,
    ///   <c>false</c> otherwise</returns>
    internal bool TryGetExpiredTimer([MaybeNullWhen(false)] out WlTimeoutTimer expiredTimer, long now)
    {
        if (m_ActiveTimers.First != null)
        {
            if (now > m_ActiveTimers.First.Value.Timeout)
            {
                expiredTimer = m_ActiveTimers.First.Value;
                StopTimer(expiredTimer);
                return true;
            }
            else
            {
                expiredTimer = null;
                return false;
            }
        }
        expiredTimer = null;
        return false;
    }

    internal long GetNextTimeout()
    {
        return m_ActiveTimers.First?.Value.Timeout ?? long.MaxValue;
    }

    #endregion

    #region Actions with timers

    /// <summary>
    /// Starts timer. Adds it to the active timers list.
    /// </summary>
    /// <remarks>
    /// set Timeout = Now + delay
    /// timer MUST be stopped before
    /// </remarks>
    internal void StartTimer(WlTimeoutTimer timer)
    {
        timer.Timeout = ReactorUtil.GetCurrentTimeMilliSecond() + GroupDelay;
        timer.IsActive = true;
        m_ActiveTimers.AddLast(timer.m_Node);

        if (m_ActiveTimers.Count == 1)
            m_TimerManager.UpdateNextTimeout(timer);
    }

    /// <summary>
    /// Stop timer, i.e. when response arrives on time.
    /// </summary>
    /// <remarks>
    /// A timer that was stopped before expiration won't time-out.
    /// Timer is removed from the active timers list.
    /// </remarks>
    internal void StopTimer(WlTimeoutTimer timer)
    {
        if (!timer.IsActive)
            return;

        if (timer.m_Node.List == m_ActiveTimers)
            m_ActiveTimers.Remove(timer.m_Node);
        timer.IsActive = false;

        m_TimerManager.OnTimerStopped(timer);
    }

    #endregion

}
