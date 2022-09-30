/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Net.Sockets;

namespace Refinitiv.Eta.ValueAdd.Common
{
    public class Notifier
    {
        private List<Socket> m_ReadSockets;
        private List<Socket> m_WriteSockets;
        private List<Socket> m_ExceptSockets;

        public List<NotifierEvent> NotifierEvents { get; private set; }
        public List<NotifierEvent> RegisteredEvents { get; private set; }

        public Notifier(int maxEventsHint)
        {
            NotifierEvents = new List<NotifierEvent>(maxEventsHint);
            RegisteredEvents = new List<NotifierEvent>(maxEventsHint);
            m_ReadSockets = new List<Socket>(maxEventsHint);
            m_WriteSockets = new List<Socket>(maxEventsHint);
            m_ExceptSockets = new List<Socket>(maxEventsHint);
        }

        public void Clear()
        {
            NotifierEvents.Clear();
            RegisteredEvents.Clear();
            m_ReadSockets.Clear();
            m_WriteSockets.Clear();
        }

        public int AddEvent(NotifierEvent notifierEvent, Socket socket, Object? userSpec)
        {
            notifierEvent.EventSocket = socket;
            notifierEvent.UserSpec = userSpec;
            notifierEvent.Notifier = this;

            RegisteredEvents.Add(notifierEvent);

            return 0;
        }

        public int RemoveEvent(NotifierEvent notifierEvent)
        {
            notifierEvent._RegisteredFlags = 0;

            RegisteredEvents.Remove(notifierEvent);

            return 0;
        }

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

        public static int RegisterRead(NotifierEvent notifierEvent)
        {
            notifierEvent._RegisteredFlags |= NotifierEventFlag.READ;
            return 0;
        }

        public static int UnRegisterRead(NotifierEvent notifierEvent)
        {
            notifierEvent._RegisteredFlags &= ~NotifierEventFlag.READ;
            return 0;
        }

        public static int RegisterWrite(NotifierEvent notifierEvent)
        {
            notifierEvent._RegisteredFlags |= NotifierEventFlag.WRITE;
            return 0;
        }

        public static int UnRegisterWrite(NotifierEvent notifierEvent)
        {
            notifierEvent._RegisteredFlags &= ~NotifierEventFlag.WRITE;
            return 0;
        }

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
