/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Net;
using System.Net.Sockets;

namespace Refinitiv.Eta.ValueAdd.Common
{
    public class EventSignal
    {
        private Socket[] m_Sockets = new Socket[2];
        private int m_ByteWritten;
        private readonly byte[] sendData = { 1 };

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

        public void Clear()
        {
            m_Sockets = new Socket[2];
        }

        public Socket GetEventSignalSocket()
        {
            return m_Sockets[0];
        }

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

        public int SetEventSignal()
        {
            if (m_ByteWritten != 0)
                return 0;

            int retVal = 0;
            do
            {
                retVal = m_Sockets[1].Send(sendData, 0, 1, SocketFlags.None, out SocketError error);
                if (retVal == 0 && error != SocketError.WouldBlock)
                    return -1;

            } while (retVal == 0);

            m_ByteWritten = 1;

            return 0;
        }

        public int ResetEventSignal()
        {
            if (m_ByteWritten == 0)
                return 0;

            int retVal;
            byte[] dummyBuffer = new byte[1];

            do
            {
                retVal = m_Sockets[0].Receive(dummyBuffer, 0, 1, SocketFlags.None, out SocketError error);
                if (retVal == 0 && error != SocketError.WouldBlock)
                    return -1;

            } while (retVal == 0);

            m_ByteWritten = 0;

            return 0;
        }
    }
}
