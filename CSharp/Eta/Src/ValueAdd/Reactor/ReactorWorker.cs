/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Common;
using System.Net.Sockets;
using SessionState = LSEG.Eta.ValueAdd.Reactor.ReactorTokenSession.SessionState;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class ReactorWorker
    {
        private const int DEFAULT_WAIT_TIME = 100; // milliseconds

        Thread? m_Thread;
        private ReactorEventQueue m_WorkerEventQueue; /* Worker's event quque */
        private ReactorEventQueue m_ReactorEventQueue; /* Reactor's event queue */
        private Reactor m_Reactor;
        private Notifier m_Notifier;
        private NotifierEvent? m_NotifierEvent;
        private InProgInfo m_InProgInfo;

        private VaIteratableQueue m_InitChannelQueue = new VaIteratableQueue();
        private VaIteratableQueue m_ActiveChannelQueue = new VaIteratableQueue();
        private VaIteratableQueue m_ReconnectingChannelQueue = new VaIteratableQueue();
        private VaIteratableQueue m_TokenSessionQueue = new VaIteratableQueue();

        private volatile bool m_Exit;

        private Socket? EventSocket { get; set; }

        internal bool IsWorkerThreadStarted { get; set; } = false;

        private long LastRecordedTimeMs { get; set; }

        private int SleepTimeMs { get; set; }

        private bool IsCalculatedWaitTime { get; set; } = false;

        internal ReactorEventQueue WorkerEventQueue
        {
            get
            {
                return m_WorkerEventQueue;
            }
        }

        public ReactorWorker(Reactor reactor)
        {
            m_Reactor = reactor;
            m_Notifier = new Notifier(1024);
            m_WorkerEventQueue = new ReactorEventQueue();
            m_InProgInfo = new InProgInfo();
            m_ReactorEventQueue = reactor.GetReactorEventQueue();
            SleepTimeMs = DEFAULT_WAIT_TIME;
        }

        public ReactorReturnCode InitReactorWorker(out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            if (m_WorkerEventQueue.InitReactorEventQueue() != ReactorReturnCode.SUCCESS)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorWorker.InitReactorWorker", "Failed to initilize event queue.");
            }

            EventSocket = m_WorkerEventQueue.GetEventQueueSocket();

            m_NotifierEvent = new NotifierEvent
            {
                _RegisteredFlags = NotifierEventFlag.READ
            };

            m_Notifier.AddEvent(m_NotifierEvent, EventSocket, m_WorkerEventQueue);

            m_Thread = new Thread(new ThreadStart(Run));
            m_Thread.Start();

            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode UnInitReactorWorker()
        {
            if (m_NotifierEvent != null)
            {
                m_Notifier.Clear();
            }

            m_WorkerEventQueue.UninitReactorEventQueue();

            return ReactorReturnCode.SUCCESS;
        }

        private void Run()
        {
            IsWorkerThreadStarted = true;
            int ret;
            m_Exit = false;

            do
            {
                if(!IsCalculatedWaitTime || SleepTimeMs < 0)
                {
                    SleepTimeMs = DEFAULT_WAIT_TIME;
                }

                ret = m_Notifier.Wait(SleepTimeMs * 1000);
                LastRecordedTimeMs = ReactorUtil.GetCurrentTimeMilliSecond();
                IsCalculatedWaitTime = false;

                if (ret > 0)
                {
                    NotifierEvent notifierEvent;
                    ReactorEvent? reactorEvent;
                    for (int i = 0; i < m_Notifier.NotifierEvents.Count; i++)
                    {
                        notifierEvent = m_Notifier.NotifierEvents[i];

                        if (notifierEvent.IsBadSocket())
                            continue;

                        if (notifierEvent.IsReadable())
                        {
                            if (notifierEvent.UserSpec == m_WorkerEventQueue)
                            {
                                /* This is events to be handled by the worker thread*/
                                reactorEvent = m_WorkerEventQueue.GetEventFromQueue();

                                ProcessReactorEventImpl(reactorEvent);
                            }
                            else
                            {
                                var reactorChannel = (ReactorChannel?)notifierEvent.UserSpec;
                                if (reactorChannel?.Channel != null && reactorChannel.State == ReactorChannelState.INITIALIZING &&
                                (reactorChannel.Channel.State == ChannelState.INACTIVE || reactorChannel.Channel.State == ChannelState.INITIALIZING))
                                {
                                    InitializeChannel(reactorChannel);
                                }
                            }
                        }
                        if(notifierEvent.IsWriteable())
                        {
                            ProcessChannelFlush((ReactorChannel?)notifierEvent.UserSpec);
                        }
                    }
                }

                if (m_Reactor.m_TimeoutTimerManager.DetectExpiredTimers(out long nextTimerMs))
                {
                    // there are some timers that have expired, Reactor must be woken
                    // up to invoke timeout callbacks
                    SendReactorImplEvent(null, ReactorEventImpl.ImplType.WATCHLIST_TIMEOUT,
                                         ReactorReturnCode.SUCCESS, "Worker.Run()",
                                         "Timer timed out");
                }
                if (nextTimerMs != Int64.MaxValue)
                    CalculateNextTimeout(nextTimerMs - LastRecordedTimeMs);

                // initialize channels and check if initialization timeout occurred
                m_InitChannelQueue.Rewind();
                while (m_InitChannelQueue.HasNext())
                {
                    ReactorChannel? reactorChannel = (ReactorChannel?)m_InitChannelQueue.Next();
                    // handle initialization timeout
                    if (reactorChannel != null && reactorChannel.State == ReactorChannelState.INITIALIZING)
                    {
                        if (reactorChannel.Channel != null &&
                            (reactorChannel.Channel.State == ChannelState.INACTIVE || reactorChannel.Channel.State == ChannelState.INITIALIZING))
                        {
                            InitializeChannel(reactorChannel);
                        }
                    }
                }

                // handle pings
                m_ActiveChannelQueue.Rewind();
                while (m_ActiveChannelQueue.HasNext())
                {
                    ReactorChannel? reactorChannel = (ReactorChannel?)m_ActiveChannelQueue.Next();
                    if (reactorChannel != null)
                    {
                        if (reactorChannel.Channel != null && reactorChannel.Channel.State == ChannelState.ACTIVE
                            && reactorChannel.State != ReactorChannelState.DOWN_RECONNECTING
                            && reactorChannel.State != ReactorChannelState.DOWN
                            && reactorChannel.State != ReactorChannelState.CLOSED
                            && reactorChannel.State != ReactorChannelState.RDP_RT
                            && reactorChannel.State != ReactorChannelState.RDP_RT_DONE
                            && reactorChannel.State != ReactorChannelState.RDP_RT_FAILED)
                        {
                            if (reactorChannel.GetPingHandler().HandlePings(reactorChannel, out Error? error)
                                < TransportReturnCode.SUCCESS)
                            {
                                reactorChannel.State = ReactorChannelState.DOWN;
                                SendReactorImplEvent(reactorChannel, ReactorEventImpl.ImplType.CHANNEL_DOWN,
                                        ReactorReturnCode.FAILURE, "Worker.Run()",
                                        "Ping error for channel: " + error?.Text);
                            }
                        }
                    }
                }

                // handle connection recovery (only for client connections)
                m_ReconnectingChannelQueue.Rewind();
                while (m_ReconnectingChannelQueue.HasNext())
                {
                    ReactorChannel? reactorChannel = (ReactorChannel?)m_ReconnectingChannelQueue.Next();

                    if (reactorChannel != null)
                    {
                        if (reactorChannel.NextRecoveryTime > LastRecordedTimeMs)
                        {
                            CalculateNextTimeout(reactorChannel.NextRecoveryTime - LastRecordedTimeMs);
                            continue;
                        }

                        IChannel? channel = null;
                        Error? error = null;

                        if(reactorChannel.State != ReactorChannelState.RDP_RT &&
                            reactorChannel.State != ReactorChannelState.RDP_RT_DONE &&
                            reactorChannel.State != ReactorChannelState.RDP_RT_FAILED)
                        {
                            channel = reactorChannel.Reconnect(out error);
                        }

                        if(reactorChannel.State == ReactorChannelState.RDP_RT ||
                            reactorChannel.State == ReactorChannelState.RDP_RT_DONE ||
                            reactorChannel.State == ReactorChannelState.RDP_RT_FAILED)
                        {
                            channel = reactorChannel.ReconnectRDP(out error);
                        }

                        if (channel is null && reactorChannel.State != ReactorChannelState.RDP_RT)
                        {
                            reactorChannel.SetChannel(channel);

                            // Reconnect attempt failed -- send channel down event.
                            m_ReconnectingChannelQueue.Remove(reactorChannel);
                            if (reactorChannel.TokenSession != null && (reactorChannel.TokenSession.SessionMgntState == SessionState.STOP_TOKEN_REQUEST ||
                                reactorChannel.TokenSession.SessionMgntState == SessionState.STOP_QUERYING_SERVICE_DISCOVERY))
                            {
                                /* This is a terminal state.  There was an REST error that we cannot recover from, so set the reconnectLimit to 0 */
                                reactorChannel.ConnectOptions!.SetReconnectAttempLimit(0);
                            }

                            SendReactorImplEvent(reactorChannel, ReactorEventImpl.ImplType.CHANNEL_DOWN,
                                    ReactorReturnCode.FAILURE, "Worker.Run()",
                                    "Reconnection failed: " + error?.Text);
                            continue;
                        }

                        if (reactorChannel.State != ReactorChannelState.RDP_RT)
                        {
                            reactorChannel.SetChannel(channel);
                            reactorChannel.State = ReactorChannelState.INITIALIZING;
                            m_ReconnectingChannelQueue.Remove(reactorChannel);

                            ProcessChannelInit(reactorChannel);
                        }
                    }
                }

                // Handles the session management to renew the access token
                m_TokenSessionQueue.Rewind();
                while (m_TokenSessionQueue.HasNext())
                {
                    ReactorTokenSession? tokenSession = (ReactorTokenSession?)m_TokenSessionQueue.Next();
                    if(tokenSession is not null)
                    {
                        if(tokenSession.GetNextAuthTokenExpireTime() < LastRecordedTimeMs)
                        {
                            tokenSession.SessionMgntState = SessionState.AUTHENTICATE_USING_CLIENT_CRED;

                             /* Clears the current access token information as it is about to expire and Reactor should get a
                             one when establising a connection */
                            tokenSession.ReactorAuthTokenInfo.Clear();

                            /* Removes from the queue as the access token is expired */
                            m_TokenSessionQueue.Remove(tokenSession);
                            tokenSession.InTokenSessionQueue = false;
                        }
                        else
                        {
                            CalculateNextTimeout(tokenSession.GetNextAuthTokenExpireTime() - LastRecordedTimeMs);
                        }
                    }
                }

            } while (!m_Exit);

            Shutdown();

            UnInitReactorWorker();

            IsWorkerThreadStarted = false;
        }

        private void ProcessReactorEventImpl(ReactorEvent? reactorEvent)
        {
            if (reactorEvent != null)
            {
                ReactorEventImpl eventImpl = (ReactorEventImpl)reactorEvent;
                ReactorChannel? reactorChannel = reactorEvent.ReactorChannel;

                switch (eventImpl.EventImplType)
                {
                    case ReactorEventImpl.ImplType.CHANNEL_INIT:
                        {
                            ProcessChannelInit(reactorChannel);
                            break;
                        }
                    case ReactorEventImpl.ImplType.CHANNEL_DOWN:
                        {
                            ProcessChannelClose(reactorChannel);

                            if (reactorChannel != null && !reactorChannel.IsClosedAckSent  &&
                                reactorChannel.Server == null && !reactorChannel.RecoveryAttemptLimitReached())
                            {
                                /* Go into connection recovery. */
                                reactorChannel.CalculateNextReconnectTime();

                                m_ReconnectingChannelQueue.Add(reactorChannel);
                            }
                            break;
                        }
                    case ReactorEventImpl.ImplType.CHANNEL_CLOSE:
                        {
                            ProcessChannelClose(reactorChannel);
                            
                            if(reactorChannel != null)
                                reactorChannel.IsClosedAckSent = true;

                            SendReactorImplEvent(reactorChannel!, ReactorEventImpl.ImplType.CHANNEL_CLOSE_ACK,
                                    ReactorReturnCode.SUCCESS, null, null);
                            break;
                        }
                    case ReactorEventImpl.ImplType.SHUTDOWN:
                        {
                            m_Exit = true;
                            break;
                        }
                    case ReactorEventImpl.ImplType.FLUSH:
                        {
                            ProcessChannelFlush(reactorChannel);
                            break;
                        }
                    case ReactorEventImpl.ImplType.FD_CHANGE:
                        {
                            ProcessChannelFDChange(reactorChannel);
                            break;
                        }
                    case ReactorEventImpl.ImplType.TOKEN_MGNT:
                        {
                            ReactorTokenSession? tokenSession = eventImpl.TokenSession;

                            if (tokenSession is not null)
                            {
                                if (!tokenSession.InTokenSessionQueue)
                                {
                                    tokenSession.InTokenSessionQueue = true;
                                    m_TokenSessionQueue.Add(tokenSession);
                                }

                                tokenSession.CalculateNextAuthTokenExpireTime(tokenSession.ReactorAuthTokenInfo.ExpiresIn);
                                long nextExpiretTime = tokenSession.GetNextAuthTokenExpireTime();

                                CalculateNextTimeout(nextExpiretTime - LastRecordedTimeMs);
                            }

                            break;
                        }
                    default:
                        System.Console.WriteLine("Worker.processWorkerEvent(): received unexpected eventType=" + eventImpl.EventImplType);
                        break;
                }

                reactorEvent.ReturnToPool();
            }
        }

        private void ProcessChannelInit(ReactorChannel? reactorChannel)
        {
            if (reactorChannel != null)
            {
                m_InitChannelQueue.Add(reactorChannel);

                if (reactorChannel.NotifierEvent != null && reactorChannel.Channel != null)
                {
                    m_Notifier.AddEvent(reactorChannel.NotifierEvent, reactorChannel.Channel.Socket, reactorChannel);
                }
            }
        }

        private void ProcessChannelClose(ReactorChannel? reactorChannel)
        {
            if (reactorChannel is null)
                return;
            if (reactorChannel.Channel != null && reactorChannel.Channel.State != ChannelState.INACTIVE)
            {
                // sckt.close will implicitly cancel any registered keys.
                reactorChannel.Channel.Close(out Error error);
                reactorChannel.FlushRequested = false;
                CancelRegister(reactorChannel);
            }

            if (m_ActiveChannelQueue.Remove(reactorChannel) == false)
                if (m_InitChannelQueue.Remove(reactorChannel) == false)
                {
                    m_ReconnectingChannelQueue.Remove(reactorChannel);
                }
        }

        private void ProcessChannelFlush(ReactorChannel? reactorChannel)
        {
            if (reactorChannel is null)
                return;

            IChannel? channel = reactorChannel.Channel;
            if (channel != null && channel.State != ChannelState.INACTIVE && channel.State != ChannelState.CLOSED)
            {
                // attempt to flush
                TransportReturnCode retval = channel.Flush(out Error error);
                if (retval > TransportReturnCode.SUCCESS)
                {
                    // flush returned positive, register this channel with the
                    // Add the Notifier object to the Notifier in order to flush later.
                    reactorChannel.NotifierEvent._RegisteredFlags |= NotifierEventFlag.WRITE;
                    m_Notifier.AddEvent(reactorChannel.NotifierEvent, channel.Socket, reactorChannel);
                }
                else if (retval == TransportReturnCode.SUCCESS)
                {
                    // flush succeeded
                    if((reactorChannel.NotifierEvent._RegisteredFlags & NotifierEventFlag.WRITE) != 0)
                    {
                        reactorChannel.NotifierEvent._RegisteredFlags &= ~NotifierEventFlag.WRITE;

                        if(reactorChannel.NotifierEvent._RegisteredFlags == 0)
                        {
                            m_Notifier.RemoveEvent(reactorChannel.NotifierEvent);
                        }
                    }

                    SendReactorImplEvent(reactorChannel, ReactorEventImpl.ImplType.FLUSH_DONE, ReactorReturnCode.SUCCESS, null, null);
                }
                else if (retval < TransportReturnCode.SUCCESS)
                {
                    if (retval != TransportReturnCode.WRITE_FLUSH_FAILED && retval != TransportReturnCode.WRITE_CALL_AGAIN)
                    {
                        // flush failed. Close this reactorChannel.
                        if (reactorChannel.State != ReactorChannelState.CLOSED && reactorChannel.State != ReactorChannelState.DOWN
                            && reactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                        {
                            reactorChannel.State = ReactorChannelState.DOWN;
                            SendReactorImplEvent(reactorChannel, ReactorEventImpl.ImplType.CHANNEL_DOWN,
                                    ReactorReturnCode.FAILURE, "Worker.ProcessChannelFlush",
                                    "failed to flush, errorId=" + error?.ErrorId
                                    + " errorText=" + error?.Text);
                        }
                    }
                }
            }
        }

        private void ProcessChannelFDChange(ReactorChannel? reactorChannel)
        {
            // cancel old reactorChannel select
            try
            {
                if(reactorChannel!.NotifierEvent.EventSocket == reactorChannel.OldSocket)
                    m_Notifier.RemoveEvent(reactorChannel.NotifierEvent);
            }
            catch (Exception)
            {
            } // old channel may be null so ignore

            // register selector with channel event's new reactorChannel
            try
            {
                if (reactorChannel!.NotifierEvent.EventSocket != reactorChannel.Socket)
                {
                    m_Notifier.AddEvent(reactorChannel.NotifierEvent, reactorChannel!.Socket!, reactorChannel);
                }
            }
            catch (Exception)
            {
                // selector register failed for this reactorChannel.
                // Close this reactorChannel.
                if (reactorChannel!.State != ReactorChannelState.CLOSED && reactorChannel.State != ReactorChannelState.DOWN
                    && reactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                {
                    reactorChannel.State = ReactorChannelState.DOWN;

                    SendReactorImplEvent(reactorChannel, ReactorEventImpl.ImplType.CHANNEL_DOWN,
                            ReactorReturnCode.FAILURE, "Worker.ProcessChannelFDChange",
                            "Notifier register failed.");
                }
            }
        }

        private void InitializeChannel(ReactorChannel reactorChannel)
        {
            IChannel? channel = reactorChannel.Channel;
            TransportReturnCode retval = channel!.Init(m_InProgInfo, out Error error);

            if (retval < TransportReturnCode.SUCCESS)
            {
                CancelRegister(reactorChannel);
                reactorChannel.State = ReactorChannelState.DOWN;
                SendReactorImplEvent(reactorChannel, ReactorEventImpl.ImplType.CHANNEL_DOWN,
                        ReactorReturnCode.FAILURE, "Worker.InitializeChannel",
                        "Error initializing channel: errorId=" + error?.ErrorId + " text="
                        + error?.Text);
                return;
            }

            switch (retval)
            {
                case TransportReturnCode.CHAN_INIT_IN_PROGRESS:
                    if (m_InProgInfo.Flags == InProgFlags.SCKT_CHNL_CHANGE)
                    {
                        if ((retval = ReRegister(m_InProgInfo, reactorChannel, out Error? reReisterErr)) != TransportReturnCode.SUCCESS)
                        {
                            CancelRegister(reactorChannel);
                            SendReactorImplEvent(reactorChannel, ReactorEventImpl.ImplType.CHANNEL_DOWN,
                                    ReactorReturnCode.FAILURE, "Worker.InitializeChannel",
                                    "Error - failed to re-register on SCKT_CHNL_CHANGE: "
                                    + reReisterErr?.Text);
                        }
                    }
                    else
                    {
                        // check if initialization timeout occurred.
                        if (ReactorUtil.GetCurrentTimeMilliSecond() > reactorChannel.InitializationEndTimeMs())
                        {
                            CancelRegister(reactorChannel);
                            SendReactorImplEvent(reactorChannel, ReactorEventImpl.ImplType.CHANNEL_DOWN,
                                    ReactorReturnCode.FAILURE, "Worker.InitializeChannel",
                                    "Error - exceeded initialization timeout ("
                                    + reactorChannel.InitializationTimeout() + " s)");
                        }
                    }

                    break;
                case TransportReturnCode.SUCCESS:
                    // init is complete, cancel selector registration.
                    CancelRegister(reactorChannel);

                    // channel.init is complete,
                    // save the channel's negotiated ping timeout
                    reactorChannel.GetPingHandler().InitPingHandler(channel.PingTimeOut);
                    reactorChannel.ResetCurrentChannelRetryCount();

                    // move the channel from the initQueue to the activeQueue
                    m_InitChannelQueue.Remove(reactorChannel);
                    m_ActiveChannelQueue.Add(reactorChannel);
                    SendReactorImplEvent(reactorChannel, ReactorEventImpl.ImplType.CHANNEL_UP,
                            ReactorReturnCode.SUCCESS, null, null);
                    break;
                default:
                    CancelRegister(reactorChannel);
                    SendReactorImplEvent(reactorChannel, ReactorEventImpl.ImplType.CHANNEL_DOWN,
                            ReactorReturnCode.FAILURE, "Worker.InitializeChannel",
                            "Error - invalid return code: " + retval);
                    break;
            }
        }

        private TransportReturnCode ReRegister(InProgInfo m_InProgInfo, ReactorChannel reactorChannel, out Error? error)
        {
            error = null;

            if(reactorChannel.NotifierEvent.EventSocket == m_InProgInfo.OldSocket)
                m_Notifier.RemoveEvent(reactorChannel.NotifierEvent);

            reactorChannel.NotifierEvent._RegisteredFlags = NotifierEventFlag.READ;

            if (reactorChannel.Channel != null)
            {
                m_Notifier.AddEvent(reactorChannel.NotifierEvent, reactorChannel.Channel.Socket, reactorChannel);

                reactorChannel.Socket = reactorChannel.Channel.Socket;
                reactorChannel.OldSocket = m_InProgInfo.OldSocket;

                return TransportReturnCode.SUCCESS;
            }
            else
            {
                return TransportReturnCode.FAILURE;
            }
        }

        private void CancelRegister(ReactorChannel reactorChannel)
        {
            m_Notifier.RemoveEvent(reactorChannel.NotifierEvent);
        }

        private void Shutdown()
        {
            foreach (var notifierEvent in m_Notifier.RegisteredEvents)
            {
                if (notifierEvent.UserSpec is ReactorChannel)
                {
                    ReactorChannel reactorChannel = (ReactorChannel)notifierEvent.UserSpec;
                    if (reactorChannel.Channel != null)
                    {
                        reactorChannel.Channel.Close(out Error error);
                        if (m_ActiveChannelQueue.Remove(reactorChannel) == false)
                            if (m_InitChannelQueue.Remove(reactorChannel) == false)
                                m_ReconnectingChannelQueue.Remove(reactorChannel);
                    }

                    reactorChannel.ReturnToPool();
                }
            }

            while (m_InitChannelQueue.Size() > 0)
            {
                ReactorChannel? reactorChannel = (ReactorChannel?)m_InitChannelQueue.Poll();
                if (reactorChannel != null)
                {
                    if (reactorChannel.Channel != null)
                    {
                        reactorChannel.Channel.Close(out Error error);
                    }

                    reactorChannel.ReturnToPool();
                }
            }

            while (m_ActiveChannelQueue.Size() > 0)
            {
                ReactorChannel? reactorChannel = (ReactorChannel?)m_ActiveChannelQueue.Poll();
                if (reactorChannel != null)
                {
                    if (reactorChannel.Channel != null)
                    {
                        reactorChannel.Channel.Close(out Error error);
                    }

                    reactorChannel.ReturnToPool();
                }
            }

            while (m_ReconnectingChannelQueue.Size() > 0)
            {
                ReactorChannel? reactorChannel = (ReactorChannel?)m_ReconnectingChannelQueue.Poll();
                if (reactorChannel != null)
                {
                    if (reactorChannel.Channel != null)
                    {
                        reactorChannel.Channel.Close(out Error error);
                    }

                    reactorChannel.ReturnToPool();
                }
            }

            while(m_TokenSessionQueue.Size() > 0)
            {
                ReactorTokenSession? reactorTokenSession = (ReactorTokenSession?)m_TokenSessionQueue.Poll();
                if(reactorTokenSession != null)
                {
                    // Just remove from the queue if any.
                }
            }
        }

        /* Send a ReactorImplEvent to Reactor*/
        private void SendReactorImplEvent(ReactorChannel? reactorChannel, ReactorEventImpl.ImplType eventType, ReactorReturnCode reactorReturnCode,
            string? location, string? text)
        {
            ReactorEventImpl reactorEventImpl = m_Reactor.GetReactorPool().CreateReactorEventImpl();
            reactorEventImpl.EventImplType = eventType;
            reactorEventImpl.ReactorChannel = reactorChannel;
            reactorEventImpl.ReactorErrorInfo.Code = reactorReturnCode;
            reactorEventImpl.ReactorErrorInfo.Error.ErrorId = (TransportReturnCode)reactorReturnCode;
            if(location != null)
            {
                reactorEventImpl.ReactorErrorInfo.Location = location;
            }

            if(text != null)
            {
                reactorEventImpl.ReactorErrorInfo.Error.Text = text;
            }

            m_ReactorEventQueue.PutEventToQueue(reactorEventImpl);
        }

        private void CalculateNextTimeout(long newTimeoutMilliseconds)
        {
            IsCalculatedWaitTime = true;
            if(newTimeoutMilliseconds < SleepTimeMs)
            {
                SleepTimeMs = (int)newTimeoutMilliseconds;
            }
        }
    }
}
