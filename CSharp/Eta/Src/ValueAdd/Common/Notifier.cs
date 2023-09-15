/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Net.Sockets;

namespace LSEG.Eta.ValueAdd.Common
{
    /// <summary>
    ///  This class is used to register <c>Socket</c>'s read, write and exception notifications.
    /// </summary>
    public class Notifier
    {
        private List<Socket> m_ReadSockets;
        private List<Socket> m_WriteSockets;
        private List<Socket> m_ExceptSockets;

        /// <summary>
        /// Gets a list of notification events to be processed by users.
        /// </summary>
        public List<NotifierEvent> NotifierEvents { get; private set; }

        /// <summary>
        /// Gets a list of registered notification events to check for Socket's notifications.
        /// </summary>
        public List<NotifierEvent> RegisteredEvents { get; private set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="Notifier"/> class by specifying a hint
        /// for the maximum number of events.
        /// </summary>
        /// <param name="maxEventsHint">A hint value for number of events.</param>
        public Notifier(int maxEventsHint)
        {
            NotifierEvents = new List<NotifierEvent>(maxEventsHint);
            RegisteredEvents = new List<NotifierEvent>(maxEventsHint);
            m_ReadSockets = new List<Socket>(maxEventsHint);
            m_WriteSockets = new List<Socket>(maxEventsHint);
            m_ExceptSockets = new List<Socket>(maxEventsHint);
        }

        /// <summary>
        /// Clears to default values.
        /// </summary>
        public void Clear()
        {
            NotifierEvents.Clear();
            RegisteredEvents.Clear();
            m_ReadSockets.Clear();
            m_WriteSockets.Clear();
        }

        /// <summary>
        /// Adds <see cref="NotifierEvent"/> to the <see cref="Notifier"/>.
        /// </summary>
        /// <param name="notifierEvent">The event to register notifications</param>
        /// <param name="socket">The <c>Socket</c> to receive notification from</param>
        /// <param name="userSpec">The user specified object.</param>
        /// <returns><c>0</c> indicates success otherwise <c>-1</c> to indicate duplication</returns>
        public int AddEvent(NotifierEvent notifierEvent, Socket socket, Object? userSpec)
        {
            notifierEvent.EventSocket = socket;
            notifierEvent.UserSpec = userSpec;
            notifierEvent.Notifier = this;

            if (RegisteredEvents.IndexOf(notifierEvent) == -1)
            {
                RegisteredEvents.Add(notifierEvent);
            }
            else
            {
                return -1;
            }

            return 0;
        }

        /// <summary>
        /// Removes <see cref="NotifierEvent"/> from the <see cref="Notifier"/>.
        /// </summary>
        /// <param name="notifierEvent">The event to unregister from notifications</param>
        /// <returns><c>0</c> indicates success otherwise <c>-1</c> if the even was not found.</returns>
        public int RemoveEvent(NotifierEvent notifierEvent)
        {
            notifierEvent._RegisteredFlags = 0;

            if(RegisteredEvents.Remove(notifierEvent))
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }

        /// <summary>
        /// Updates the <c>Socket</c> associated with an event.
        /// </summary>
        /// <param name="notifierEvent">The event to update</param>
        /// <param name="socket">The <c>Socket</c> to update wit the event</param>
        /// <returns><c>0</c> indicates success otherwise <c>-1</c> if the even was not found.</returns>
        public int UpdateEventSocket(NotifierEvent notifierEvent, Socket socket)
        {
            notifierEvent._RegisteredFlags = 0;

            for(int i =0; i < RegisteredEvents.Count;i++)
            {
                if(ReferenceEquals(RegisteredEvents[i], notifierEvent))
                {
                    notifierEvent.EventSocket = socket;
                    return 0;
                }
            }

            return -1; /* Not found. */
        }

        /// <summary>
        /// Register read notification for the event.
        /// </summary>
        /// <param name="notifierEvent">The event to register</param>
        /// <returns><c>0</c> for success</returns>
        public static int RegisterRead(NotifierEvent notifierEvent)
        {
            notifierEvent._RegisteredFlags |= NotifierEventFlag.READ;
            return 0;
        }

        /// <summary>
        /// Unregister read notification from the event
        /// </summary>
        /// <param name="notifierEvent">The event to unregister</param>
        /// <returns><c>0</c> for success</returns>
        public static int UnRegisterRead(NotifierEvent notifierEvent)
        {
            notifierEvent._RegisteredFlags &= ~NotifierEventFlag.READ;
            return 0;
        }

        /// <summary>
        /// Register write notification for the event.
        /// </summary>
        /// <param name="notifierEvent">The event to register</param>
        /// <returns><c>0</c> for success</returns>
        public static int RegisterWrite(NotifierEvent notifierEvent)
        {
            notifierEvent._RegisteredFlags |= NotifierEventFlag.WRITE;
            return 0;
        }

        /// <summary>
        /// Unregister read notification from the event
        /// </summary>
        /// <param name="notifierEvent">The event to unregister</param>
        /// <returns><c>0</c> for success</returns>
        public static int UnRegisterWrite(NotifierEvent notifierEvent)
        {
            notifierEvent._RegisteredFlags &= ~NotifierEventFlag.WRITE;
            return 0;
        }

        /// <summary>
        /// Waits for notification on the notifier's associated <c>Socket</c>
        /// </summary>
        /// <param name="timeOutUsec">The wait timeout in microsecond.
        /// A value equal to <c>-1</c> indicate infinite timeout</param>
        /// <returns><c>0</c> for success or number of events to handle; otherwise <c>-1</c> for failure</returns>
        public int Wait(int timeOutUsec)
        {
            m_ReadSockets.Clear(); m_WriteSockets.Clear(); m_ExceptSockets.Clear();
            NotifierEvent notifierEvent;

            for (int i = 0; i < RegisteredEvents.Count;i++)
            {
                notifierEvent = RegisteredEvents[i];
                notifierEvent.NotifiedFlags = 0;

                if (notifierEvent.EventSocket != null)
                {
                    if ((notifierEvent._RegisteredFlags & NotifierEventFlag.READ) != 0)
                    {
                        m_ReadSockets.Add(notifierEvent.EventSocket);
                        m_ExceptSockets.Add(notifierEvent.EventSocket);
                    }

                    if ((notifierEvent._RegisteredFlags & NotifierEventFlag.WRITE) != 0)
                    {
                        m_WriteSockets.Add(notifierEvent.EventSocket);
                    }
                }
            }

            if(m_ReadSockets.Count == 0 && m_ExceptSockets.Count == 0 && m_WriteSockets.Count == 0)
            {
                Thread.Sleep((int)timeOutUsec/1000);
                return 0;
            }

            try
            {
                Socket.Select(m_ReadSockets, m_WriteSockets, m_ExceptSockets, timeOutUsec);
            }
            catch(SocketException socketExcep)
            {
                if(socketExcep.SocketErrorCode == SocketError.NotSocket)
                {
                    return SetBadSocketEvetns();
                }
                else
                {
                    return -1;
                }
            }
            catch(ObjectDisposedException)
            {
                return SetBadSocketEvetns();
            }

            NotifierEvents.Clear();
            for (int i = 0; i < RegisteredEvents.Count; i++)
            {
                notifierEvent = RegisteredEvents[i];

                if (notifierEvent.EventSocket != null)
                {
                    if( m_ReadSockets.IndexOf(notifierEvent.EventSocket) != -1 || m_ExceptSockets.IndexOf(notifierEvent.EventSocket) != -1)
                    {
                        notifierEvent.NotifiedFlags |= NotifierEventFlag.READ;
                    }

                    if(m_WriteSockets.IndexOf(notifierEvent.EventSocket) != -1)
                    {
                        notifierEvent.NotifiedFlags |= NotifierEventFlag.WRITE;
                    }

                    if(notifierEvent.NotifiedFlags != 0)
                    {
                        NotifierEvents.Add(notifierEvent);
                    }
                }
            }

            return NotifierEvents.Count;
        }

        private int SetBadSocketEvetns()
        {
            NotifierEvents.Clear();
            for (int i = 0; i < RegisteredEvents.Count; i++)
            {
                NotifierEvent notifierEvent = RegisteredEvents[i];
                notifierEvent.NotifiedFlags = NotifierEventFlag.BAD_SOCKET;
                NotifierEvents.Add(notifierEvent);
            }

            return NotifierEvents.Count;
        }
    }
}
