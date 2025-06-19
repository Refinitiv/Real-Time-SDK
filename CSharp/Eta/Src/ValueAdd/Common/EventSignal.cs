/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Net;
using System.Net.Sockets;

namespace LSEG.Eta.ValueAdd.Common
{
    /// <summary>
    /// This class is used to create an event signal to notify the read notification of a <c>Socket</c>.
    /// </summary>
    public class EventSignal
    {
        private Socket[] m_Sockets = new Socket[2];
        private int m_ByteWritten;
        private readonly byte[] sendData = { 1 };
        private readonly byte[] readBuffer = new byte[1];

        /// <summary>
        /// This is used to initialize this object to send a notification event.
        /// </summary>
        /// <returns><c>0</c> for success; otherwise returns <c>-1</c></returns>
        public int InitEventSignal()
        {
            Socket serverSocket;

            serverSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, System.Net.Sockets.ProtocolType.Tcp)
            {
                Blocking = true,
                NoDelay = true
            };

            IPAddress localAddress = IPAddress.Loopback;
            IPEndPoint localEp = new IPEndPoint(localAddress, 0); /* Use ephemeral port. */

            try
            {
                serverSocket.Bind(localEp);

                serverSocket.Listen(1);
            }
            catch (Exception)
            {
                serverSocket.Close();
                return -1;
            }

            IPEndPoint endPoint = (IPEndPoint)serverSocket.LocalEndPoint!;
            int port = endPoint.Port;

            Socket client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, System.Net.Sockets.ProtocolType.Tcp)
            {
                Blocking = true,
                NoDelay = true
            };

            IPEndPoint remote_ep = new IPEndPoint(endPoint.Address, port);

            Socket acceptedSocket;

            try
            {
                client.Connect(remote_ep);
                acceptedSocket = serverSocket.Accept();
            }
            catch(Exception)
            {
                serverSocket.Close();
                client.Close();
                return -1;
            }

            m_Sockets[0] = acceptedSocket;
            m_Sockets[1] = client;

            m_Sockets[0].Blocking = false;
            m_Sockets[0].NoDelay = true;
            m_Sockets[1].Blocking = false;

            serverSocket.Close();
            m_ByteWritten = 0;
            return 0;
        }

        /// <summary>
        /// Clears this object
        /// </summary>
        public void Clear()
        {
            m_Sockets = new Socket[2];
        }

        /// <summary>
        /// Gets the <c>Socket</c> to listen for an event
        /// </summary>
        /// <returns>The Socket</returns>
        public Socket GetEventSignalSocket()
        {
            return m_Sockets[0];
        }

        /// <summary>
        /// Cleans up this object to close the Sockets. The <see cref="Clear"/> must be called
        /// to initialize the event signal again.
        /// </summary>
        public void CleanupEventSignal()
        {
            if(m_Sockets[0] != null)
            {
                m_Sockets[0].Close();
                m_Sockets[0].Dispose();
            }

            if (m_Sockets[1] != null)
            {
                m_Sockets[1].Close();
                m_Sockets[1].Dispose();
            }
        }

        /// <summary>
        /// Sets an event to single the listeners.
        /// </summary>
        /// <returns><c>0</c> for success; otherwise returns <c>-1</c></returns>
        public int SetEventSignal()
        {
            if (m_ByteWritten != 0)
                return 0;

            int retVal;
            do
            {
                retVal = m_Sockets[1].Send(sendData, 0, 1, SocketFlags.None, out SocketError error);
                if (retVal == 0 && error != SocketError.WouldBlock && error != SocketError.TryAgain)
                    return -1;

            } while (retVal == 0);

            m_ByteWritten = 1;

            return 0;
        }

        /// <summary>
        /// Resets the event signal to stop read notification of the <c>Socket</c>.
        /// </summary>
        /// <returns><c>0</c> for success; otherwise returns <c>-1</c></returns>
        public int ResetEventSignal()
        {
            if (m_ByteWritten == 0)
                return 0;

            int retVal;

            do
            {
                retVal = m_Sockets[0].Receive(readBuffer, 0, 1, SocketFlags.None, out SocketError error);
                if (retVal == 0 && error != SocketError.WouldBlock && error != SocketError.TryAgain)
                    return -1;

            } while (retVal == 0);

            m_ByteWritten = 0;

            return 0;
        }
    }
}
