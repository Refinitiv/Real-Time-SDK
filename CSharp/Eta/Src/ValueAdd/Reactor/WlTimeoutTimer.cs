/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Common;

namespace LSEG.Eta.ValueAdd.Reactor;

/// <summary>
/// This is a timer object that can be <see cref="Start()"/>-started and
/// <see cref="Stop()"/>-stopped, and also stores a delegate method to be
/// <see cref="InvokeCallback()"/>-invoked upon time-out.
/// </summary>
/// <remarks>
/// A timer instance belongs to a <see cref="WlTimeoutTimerGroup"/> that actually defines
/// timeout period.
/// </remarks>
/// <seealso cref="ReactorPool.CreateWlTimeoutTimer(WlTimeoutTimerGroup, Action{WlTimeoutTimer})"/>
internal class WlTimeoutTimer : VaNode
{
    /// <summary>
    /// Callback delegate to be invoked upon timer timeout.
    /// </summary>
    /// <remarks>
    /// This function accepts a single parameter: the timer that has timed out. It then
    /// can access <see cref="UserSpecObject"/> to handle the object associated with this
    /// timer.
    /// </remarks>
    /// <seealso cref="InvokeCallback()"/>
    private Action<WlTimeoutTimer>? m_Callback;

    /// <summary>
    /// This is an optional object that can be supplied before the timer is started
    /// and then passed to the callback function for processing.
    /// </summary>
    internal Object? UserSpecObject;

    /// <summary>
    /// Node in the <see cref="WlTimeoutTimerGroup.m_ActiveTimers"/> linked
    /// list. Reference to the node is preserved along with the active timer to speed-up
    /// its removal from the list and to avoid allocation of new node instances whenever a
    /// timer is restarted (removed and added to that list).
    /// </summary>
    internal readonly LinkedListNode<WlTimeoutTimer> m_Node;

    /// <summary>
    /// Timestamp of the moment when this timer instance expires.
    /// </summary>
    /// <remarks>
    /// The value obtained using <see cref="ReactorUtil.GetCurrentTimeMilliSecond()"/>
    /// </remarks>
    internal long Timeout = long.MaxValue;

    /// <summary>
    /// Whether a timer is active.
    /// </summary>
    /// <remarks>
    /// A timer is activated when it is started, and inactivated when it is either stopped
    /// by user or expired.
    /// </remarks>
    internal bool IsActive = false;

    /// <summary>
    /// Whether the timer was stopped by user.
    /// </summary>
    /// <remarks>
    /// A timer can be inactivated because it was expired, not started, etc.
    /// But if it was stopped by user, then the timeout event callback won't be invoked.
    /// </remarks>
    /// <seealso cref="Stop()"/>
    private bool m_IsStopped = false;

    private WlTimeoutTimerGroup? m_Group;

    internal WlTimeoutTimer()
    {
        m_Node = new LinkedListNode<WlTimeoutTimer>(this);
    }

    internal WlTimeoutTimer(WlTimeoutTimerGroup timerGroup, Action<WlTimeoutTimer> callback) : this()
    {
        Init(timerGroup, callback);
    }

    internal void Init(WlTimeoutTimerGroup timerGroup, Action<WlTimeoutTimer> callback)
    {
        m_Group = timerGroup;
        m_Callback = callback;
    }

    public override void ReturnToPool()
    {
        /* Ensure that the timer is stopped before returning back to the pool */
        Stop();
        base.ReturnToPool();
        Clear();
    }

    /// <summary>
    /// Starts timer.
    /// </summary>
    /// <seealso cref="Stop()" />
    public void Start()
    {
        m_IsStopped = false;
        m_Group!.StartTimer(this);
    }

    /// <summary>
    /// Stops timer. If called before <see cref="WlTimeoutTimerGroup.GroupDelay" />,
    /// then this timer won't expire.
    /// </summary>
    public void Stop()
    {
        m_IsStopped = true;
        m_Group?.StopTimer(this);
    }

    /// <summary>
    /// Invokes callback if the timer wasn't <see cref="Stop()"/> - stopped before.
    /// </summary>
    /// <remarks>
    /// Callback can be invoked only once. Then the timer has to be restarted.
    /// </remarks>
    /// <seealso cref="Start()"/>
    /// <seealso cref="Stop()"/>
    public void InvokeCallback()
    {
        if (!m_IsStopped)
        {
            m_Callback?.Invoke(this);
            m_IsStopped = true;
        }
    }

    /// <summary>
    /// Clean-up the timer instance for reuse within the same timer group.
    /// </summary>
    public void Clear()
    {
        UserSpecObject = null;
        m_Callback = null;
        m_Group = null;
        IsActive = false;
        m_IsStopped = false;
    }
}
