/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Threading;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Common;

namespace LSEG.Ema.Access
{
    interface ITimeoutClient
    {
        void HandleTimeoutEvent();
    }

    internal class TimeoutEventManager
    {
        private LinkedList<TimeoutEvent> timeoutQueue = new();

        private long LastTimeout;
        private long FirstTimeout;

        private MonitorWriteLocker queueLock = new MonitorWriteLocker(new object());

        private IOmmCommonImpl m_OmmCommonImpl;

        public EventSignal TimeoutSignal { get; private set; }

        private volatile bool m_SingnalCleanup;

        public TimeoutEventManager(IOmmCommonImpl commonImpl, EventSignal timeoutSignal)
        {
            TimeoutSignal = timeoutSignal;
            m_OmmCommonImpl = commonImpl;
            m_SingnalCleanup = false;
        }

        public TimeoutEvent AddTimeoutEvent(long timeoutInMicroSec, ITimeoutClient timeoutClient)
        {
            queueLock.Enter();

            TimeoutEvent timeout = new(timeoutInMicroSec, timeoutClient);

            try
            {
                if (timeoutQueue.Count == 0)
                {
                    timeoutQueue.AddLast(timeout);
                    FirstTimeout = LastTimeout = timeout.ExpiredInMicroSecond;
                }
                else if (LastTimeout <= timeout.ExpiredInMicroSecond)
                {
                    timeoutQueue.AddLast(timeout);
                    LastTimeout = timeout.ExpiredInMicroSecond;
                }
                else if (FirstTimeout > timeout.ExpiredInMicroSecond)
                {
                    timeoutQueue.AddFirst(timeout);
                    FirstTimeout = timeout.ExpiredInMicroSecond;
                }
                else
                {
                    LinkedListNode<TimeoutEvent>? currentNode = timeoutQueue.Last;
                    LinkedListNode<TimeoutEvent>? prevNode;
                    TimeoutEvent currentTimeout;

                    while (currentNode != null)
                    {
                        currentTimeout = currentNode.Value;
                        prevNode = currentNode.Previous;

                        if (currentTimeout.ExpiredInMicroSecond <= timeout.ExpiredInMicroSecond)
                        {
                            timeoutQueue.AddAfter(currentNode, timeout);
                            break;
                        }

                        /* Moves to previous node if it is not the first node. */
                        if (currentNode != timeoutQueue.First)
                        {
                            currentNode = prevNode;
                        }
                        else
                        {
                            /* The timeout must be less then the first node so add it in front of the 
                             queue and update the first timeout. */
                            timeoutQueue.AddBefore(currentNode, timeout);
                            FirstTimeout = timeout.ExpiredInMicroSecond;
                            break;
                        }
                    }
                }
            }
            finally
            {
                queueLock.Exit();
            }

            if (!m_SingnalCleanup)
                TimeoutSignal.SetEventSignal();

            return timeout;
        }

        public long CheckUserTimeoutExist()
        {
            queueLock.Enter();

            try
            {
                if (timeoutQueue.Count == 0)
                {
                    return -1;
                }

                LinkedListNode<TimeoutEvent>? currentNode = timeoutQueue.First;
                LinkedListNode<TimeoutEvent>? nextNode;
                TimeoutEvent currentTimeout;

                while (currentNode != null)
                {
                    currentTimeout = currentNode.Value;
                    nextNode = currentNode.Next;

                    if (currentTimeout.Cancelled)
                    {
                        if (currentNode != timeoutQueue.Last)
                        {
                            timeoutQueue.Remove(currentNode);
                            currentNode = nextNode;
                            continue;
                        }
                        else
                        {
                            /* This is the last node so exits the loop */
                            timeoutQueue.Remove(currentNode);
                            break;
                        }
                    }

                    long currentTime = EmaUtil.GetMicroseconds();
                    if (currentTime >= currentTimeout.ExpiredInMicroSecond)
                    {
                        return 0;
                    }
                    else
                    {
                        return (currentTimeout.ExpiredInMicroSecond - currentTime);
                    }
                }

                return -1;
            }
            finally
            {
                queueLock.Exit();
            }
        }

        public void Execute()
        {
            queueLock.Enter();

            try
            {
                if (timeoutQueue.Count == 0)
                {
                    return;
                }

                LinkedListNode<TimeoutEvent>? currentNode = timeoutQueue.First;
                LinkedListNode<TimeoutEvent>? nextNode;
                TimeoutEvent currentTimeout;

                long currentTime = EmaUtil.GetMicroseconds();
                while (currentNode != null)
                {
                    currentTimeout = currentNode.Value;
                    nextNode = currentNode.Next;

                    if (currentTime >= currentTimeout.ExpiredInMicroSecond)
                    {
                        if (!currentTimeout.Cancelled)
                        {
                            m_OmmCommonImpl.EventReceived();
                            currentTimeout.Client.HandleTimeoutEvent();
                        }

                        if (currentNode != timeoutQueue.Last)
                        {
                            timeoutQueue.Remove(currentNode);
                            currentNode = nextNode;

                            /* Update the first timeout with the next node. */
                            if (currentNode != null)
                            {
                                FirstTimeout = currentNode.Value.ExpiredInMicroSecond;
                            }
                        }
                        else
                        {
                            /* This is the last node so reset the signal and exit the loop. */
                            timeoutQueue.Remove(currentNode);

                            FirstTimeout = LastTimeout = 0;

                            if (!m_SingnalCleanup)
                                TimeoutSignal.ResetEventSignal();
                            break;
                        }
                    }
                    else
                        break;
                }
            }
            finally
            {
                queueLock.Exit();
            }
        }

        public void CleanupEventSignal()
        {
            queueLock.Enter();

            try
            {
                if (!m_SingnalCleanup)
                {
                    m_SingnalCleanup = true;
                    TimeoutSignal.CleanupEventSignal();
                }
            }
            finally
            {
                queueLock.Exit();
            }
        }
    }

    internal class TimeoutEvent
    {
        internal long ExpiredInMicroSecond { get; private set; }
        
        internal bool Cancelled { get; private set; }

        internal ITimeoutClient Client { get; private set; }

        public TimeoutEvent(long timeoutInMicroSec, ITimeoutClient client)
        {
            ExpiredInMicroSecond = timeoutInMicroSec + EmaUtil.GetMicroseconds();
            Client = client;
            Cancelled = false;
        }

        public void Cancel()
        {
            Cancelled = true;
        }
    }
}
