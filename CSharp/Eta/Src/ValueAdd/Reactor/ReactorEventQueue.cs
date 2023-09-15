/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Common;
using System.Net.Sockets;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class ReactorEventQueue
    {
        EventSignal m_EventSignal;
        VaQueue m_EventQueue;
        private ReaderWriterLockSlim EventQueueLock { get; set; } = new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);

        public ReactorEventQueue()
        {
            m_EventSignal = new EventSignal();
            m_EventQueue = new VaQueue();
        }

        public ReactorReturnCode InitReactorEventQueue()
        {
            if( m_EventSignal.InitEventSignal() != 0)
            {
                return ReactorReturnCode.FAILURE;
            }

            return ReactorReturnCode.SUCCESS;
        }

        public void UninitReactorEventQueue()
        {
            m_EventSignal.CleanupEventSignal();
            EventQueueLock.Dispose();
        }

        public Socket GetEventQueueSocket()
        {
            return m_EventSignal.GetEventSignalSocket();
        }

        public void PutEventToQueue(ReactorEvent reactorEvent)
        {
            try
            {
                EventQueueLock.EnterWriteLock();
                m_EventQueue.Add(reactorEvent);
               
                /* Queue was previously empty; Needs to notify queue's listener*/
                if(m_EventQueue.Size() == 1)
                {
                    m_EventSignal.SetEventSignal();
                }
            }
            finally
            {
                EventQueueLock.ExitWriteLock();
            }
        }

        public ReactorEvent? GetEventFromQueue()
        {
            ReactorEvent? reactorEvent = null;
            try
            {
                EventQueueLock.EnterWriteLock();
                int count = m_EventQueue.Size();
                reactorEvent = (ReactorEvent?)m_EventQueue.Poll();

                /* Queue is now empty; Needs to reset queue's read notification*/
                if (count == 1)
                {
                    m_EventSignal.ResetEventSignal();
                }
            }
            finally
            {
                EventQueueLock.ExitWriteLock();
            }

            return reactorEvent;
        }

        public int GetEventQueueSize()
        {
            return (int)m_EventQueue.Size();
        }
    }
}
