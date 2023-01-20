/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;
using LSEG.Eta.Transports;
using LSEG.Eta.Codec;
using System.Net.Sockets;
using LSEG.Eta.Common;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// Encapsulate ETA transport methods for the interactive provider and generic
    /// provider example applications.
    /// </summary>
    public class ProviderSession
    {
        // client sessions over this limit gets rejected with NAK mount 
        public static readonly int NUM_CLIENT_SESSIONS = 10;

        // client sessions over this limit gets disconnected.
        public static readonly int CLIENT_SESSIONS_LIMIT = 30;

        public ClientSessionInfo[] ClientSessions { get; private set; } = new ClientSessionInfo[CLIENT_SESSIONS_LIMIT];

        private IServer? m_Server;
        private int m_ClientSessionCount = 0;
        private StringBuilder xmlString = new StringBuilder();
        private XmlTraceDump xmlTraceDump = new XmlTraceDump();
        private bool shouldXmlTrace = false;
        private DataDictionary? m_DictionaryForXml;

        private InitArgs m_InitArgs = new InitArgs();
        private InProgInfo m_InProgInfo = new InProgInfo();
        public BindOptions BindOptions { get; private set; } = new BindOptions();
        private AcceptOptions m_AcceptOptions = new AcceptOptions();
        private WriteArgs m_WriteArgs = new WriteArgs();
        private ReadArgs m_ReadArgs = new ReadArgs();
        private ChannelInfo m_ChannelInfo = new ChannelInfo();
        public bool EnableChannelWriteLocking { get; set; }

        public List<SelectElement> ClientSessionSocketList { get; private set; } = new List<SelectElement>(CLIENT_SESSIONS_LIMIT);
        private Dictionary<Socket, IChannel> m_ClientChannelDict { get; set; } = new Dictionary<Socket, IChannel>(CLIENT_SESSIONS_LIMIT);

        public Socket? ServerSocket
        {
            get
            {
                if (m_Server != null && m_Server.Socket != null)
                {
                    return m_Server.Socket;
                }
                else
                    return null;
            }
        }

        /// <summary>
        /// Gets the <see cref="IServer"/> for this provider session.
        /// </summary>
        public IServer? Server => m_Server;

        /// <summary>
        /// Instantiates a new provider session.
        /// </summary>
        public ProviderSession()
        {
            for (int i = 0; i < CLIENT_SESSIONS_LIMIT; i++)
            {
                ClientSessions[i] = new ClientSessionInfo();
            }

            BindOptions.Clear();
        }

        /// <summary>
        /// Initializes ETA transport and binds the server to a local port.
        /// </summary>
        /// <param name="globalLock">flag to enable global locking on ETA Transport</param>
        /// <param name="error">Error information when Init fails.</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        public TransportReturnCode Init(bool globalLock, out Error? error)
        {
            error = null;

            foreach (ClientSessionInfo clientSession in ClientSessions)
            {
                clientSession.Clear();
            }

            m_InitArgs.Clear();
            m_InitArgs.GlobalLocking = globalLock;
            if (Transport.Initialize(m_InitArgs, out error) != TransportReturnCode.SUCCESS)
                return TransportReturnCode.FAILURE;

            // Sets bind options
            BindOptions.GuaranteedOutputBuffers = 500;
            BindOptions.MajorVersion = Codec.Codec.MajorVersion();
            BindOptions.MinorVersion = Codec.Codec.MinorVersion();
            BindOptions.ProtocolType = (Transports.ProtocolType)Codec.Codec.ProtocolType();

            m_Server = Transport.Bind(BindOptions, out error);

            if (m_Server is null)
            {
                error.Text = $"Unable to bind server: {error.Text}";
                Transport.Uninitialize();
                return TransportReturnCode.FAILURE;
            }

            Console.WriteLine($"\nServer bound on port {m_Server.PortNumber}");

            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Closes server session and uninitialize ETAJ Transport.
        /// </summary>
        public void UnInit()
        {
            foreach (ClientSessionInfo clientSession in ClientSessions)
            {
                if (clientSession.ClientChannel != null && clientSession.ClientChannel.State == ChannelState.ACTIVE)
                {
                    RemoveChannel(clientSession.ClientChannel);
                }
            }
        }

        /// <summary>
        /// Handles new client connection.
        /// </summary>
        /// <param name="server">Server session</param>
        /// <param name="error">error information when accept fails.</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        public TransportReturnCode HandleNewClientSession(IServer server, out Error? error)
        {
            // error = null;
            m_ClientSessionCount++;
            m_AcceptOptions.Clear();

            if (m_ClientSessionCount <= NUM_CLIENT_SESSIONS)
            {
                m_AcceptOptions.NakMount = false;
            }
            else
            {
                m_AcceptOptions.NakMount = true;
            }

            m_AcceptOptions.ChannelWriteLocking = EnableChannelWriteLocking;

            IChannel clientChannel = server.Accept(m_AcceptOptions, out error);
            if (clientChannel is null)
                return TransportReturnCode.FAILURE;

            ClientSessionInfo? clientSessionInfo = null;

            // Find an available client session
            bool clientSessionFound = false;
            for (int i = 0; i < ClientSessions.Length; i++)
            {
                clientSessionInfo = ClientSessions[i];

                if (clientSessionInfo.ClientChannel is null)
                {
                    clientSessionInfo.ClientChannel = clientChannel;
                    clientSessionInfo.StartTime = System.DateTime.UtcNow.Ticks;
                    clientSessionFound = true;
                    break;
                }
                if (Object.ReferenceEquals(clientSessionInfo.ClientChannel, clientChannel))
                {
                    clientSessionInfo.StartTime = System.DateTime.UtcNow.Ticks;
                    clientSessionFound = true;
                    break;
                }
            }

            if (clientSessionFound == false)
            {
                for (int i = 0; i < ClientSessions.Length; i++)
                {
                    clientSessionInfo = ClientSessions[i];
                    if (clientSessionInfo.ClientChannel?.Socket == null ||
                        clientSessionInfo.ClientChannel.Socket.Connected == false)
                    {
                        RemoveChannel(clientSessionInfo.ClientChannel);
                        clientSessionInfo.ClientChannel = clientChannel;
                        clientSessionInfo.StartTime = System.DateTime.UtcNow.Ticks;
                        clientSessionFound = true;
                        break;
                    }
                }
            }

            // Closes the client channel if no more client sessions
            if (clientSessionFound == false)
            {
                m_ClientSessionCount--;
                Console.WriteLine($"Rejected client: {clientChannel.Socket.Handle} {m_ClientSessionCount}");
                RemoveChannel(clientChannel);
                return TransportReturnCode.SUCCESS;
            }
            else
            {
                if (clientSessionInfo != null)
                {
                    clientSessionInfo.SelectElement.Socket = clientChannel.Socket;
                    Console.WriteLine($"New client: {clientSessionInfo.SelectElement.Socket.Handle}");

                    clientSessionInfo.SelectElement.Mode = SelectMode.READ;
                    ClientSessionSocketList.Add(clientSessionInfo.SelectElement);
                    m_ClientChannelDict[clientChannel.Socket] = clientChannel;
                }
            }

            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Reads from a channel.
        /// </summary>
        /// <param name="channel">The channel to be read from</param>
        /// <param name="error">the error in case of failures</param>
        /// <param name="callback">the callback to receive messages</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        public TransportReturnCode Read(IChannel channel, out Error? error, IReceivedMsgCallback callback)
        {
            error = null;
            TransportReturnCode ret;

            if (channel.Socket != null && channel.State == ChannelState.INITIALIZING)
            {
                ret = InitChannel(channel, out error, callback);

                if(ret != TransportReturnCode.SUCCESS || channel.State == ChannelState.INITIALIZING)
                    return ret;
            }

            if(channel.Socket != null && channel.State == ChannelState.ACTIVE)
            {
                return ReadLoop(channel, out error, callback);
            }
            else if(channel.State == ChannelState.CLOSED)
            {
                Console.WriteLine("channelClosed socket="
                   + channel.Socket!.Handle + "<"
                   + error?.Text + ">");

                RemoveClientSessionForChannel(channel);
                callback.ProcessChannelClose(channel);
            }

            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Writes the content of the {@link TransportBuffer} to the ETA channel.
        /// </summary>
        /// <param name="channel">the channel</param>
        /// <param name="msgBuf">the message buffer</param>
        /// <param name="error">the error</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        public TransportReturnCode Write(IChannel channel, ITransportBuffer msgBuf, out Error? error)
        {
            error = null;
            ITransportBuffer tempBuf = msgBuf;
            if (channel is null)
            {
                error = new Error()
                {
                    Text = "Channel is null",
                    ErrorId = TransportReturnCode.FAILURE
                };
                return TransportReturnCode.FAILURE;
            }

            if (shouldXmlTrace)
            {
                xmlString.Clear();
                xmlString.Append("\nWrite message (RWF): ");
                xmlTraceDump.DumpBuffer(channel, Codec.Codec.RWF_PROTOCOL_TYPE, tempBuf, m_DictionaryForXml, xmlString, out error);
                Console.WriteLine(xmlString);
            }

            // write data to the channel
            TransportReturnCode retval = channel.Write(tempBuf, m_WriteArgs, out error);

            if (retval > TransportReturnCode.FAILURE)
            {
                SetMsgSent(channel);
                /*
                 * The write was successful and there is more data queued in ETA
                 * Transport. Use selector to be notified when output space becomes
                 * available on a connection.
                 */
                if (retval > TransportReturnCode.SUCCESS)
                    return RegForWriteNotification(channel);
            }
            else
            {
                // Handle return codes appropriately, not all return values are
                // failure conditions
                switch (retval)
                {
                    case TransportReturnCode.SUCCESS:
                        {
                            // Successful write and all data has been passed to the
                            // connection
                            // Continue with next operations.
                        }
                        break;
                    case TransportReturnCode.WRITE_CALL_AGAIN:
                        {
                            /*
                             * Large buffer is being split by transport, but out of
                             * output buffers
                             */
                            /*
                             * Schedule a call to flush and then call the write method
                             * again with this same exact buffer to continue the
                             * fragmentation process.
                             */
                            while (retval == TransportReturnCode.WRITE_CALL_AGAIN)
                            {
                                retval = channel.Flush(out error);
                                if (retval < TransportReturnCode.SUCCESS)
                                    Console.WriteLine("channel flush failed with returned code: " + retval + " - " + error.Text);
                                retval = channel.Write(tempBuf, m_WriteArgs, out error);
                            }

                            /*
                             * The write was successful and there is more data queued in
                             * ETA Transport. Use selector to be notified when output
                             * space becomes available on a connection.
                             */
                            if (retval > TransportReturnCode.SUCCESS)
                            {
                                return RegForWriteNotification(channel);
                            }
                        }
                        break;
                    case TransportReturnCode.WRITE_FLUSH_FAILED:
                        {
                            /*
                             * The write was successful, but an attempt to flush failed.
                             * ETA will release buffer.
                             */
                            if (channel.State == ChannelState.CLOSED)
                            {
                                error.Text = "Error (" + error.ErrorId + ") (errno: " + error.SysError + ") encountered with write. Error text: " + error.Text;

                                // Connection should be closed, return failure
                                return TransportReturnCode.FAILURE;
                            }
                            else
                            {

                                /*
                                 * The write was successful and there is more data
                                 * queued in ETA Transport. Use selector to be notified
                                 * when output space becomes available on a connection.
                                 */

                                return RegForWriteNotification(channel);
                            }
                        }
                    case TransportReturnCode.INIT_NOT_INITIALIZED:
                    case TransportReturnCode.FAILURE:
                        {
                            /* write failed, release buffer */
                            if (channel.ReleaseBuffer(tempBuf, out Error flushError) != TransportReturnCode.SUCCESS)
                            {
                                error.Text = flushError.Text;
                            }

                            return TransportReturnCode.FAILURE;
                        }
                    default:
                        Console.WriteLine("Unexpected return code (" + retval + ") encountered!");
                        return retval;
                }
            }

            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Flush for write file descriptor and active state.
        /// </summary>
        /// <param name="socket">The socket to flush</param>
        /// <param name="error">The error in case of failures</param>
        public void Flush(Socket socket, out Error? error)
        {
            error = null;

            ClientSessionInfo? clientSessionInfo = GetClientSessionForChannel(socket);

            if (clientSessionInfo != null && clientSessionInfo.ClientChannel != null)
            {
                // flush the data
                TransportReturnCode ret = clientSessionInfo.ClientChannel.Flush(out error);
                if (ret < TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine("Flush() failed with return code: " + error.Text);
                }
                else if (ret == TransportReturnCode.SUCCESS)
                {
                    // remove write notification
                    RemoveOption(clientSessionInfo.ClientChannel, SelectMode.WRITE);

                }
            }
        }

        /// <summary>
        /// Gets a client session for a channel.
        /// </summary>
        /// <param name="channel">The channel to get a client session</param>
        /// <returns><see cref="ClientSessionInfo"/></returns>
        public ClientSessionInfo? GetClientSessionForChannel(IChannel channel)
        {
            foreach (ClientSessionInfo clientSessionInfo in ClientSessions)
            {
                if (clientSessionInfo.ClientChannel == channel)
                {
                    return clientSessionInfo;
                }
            }

            return null;
        }

        /// <summary>
        /// Gets a client session for a socket.
        /// </summary>
        /// <param name="socket">The socket to get a client session</param>
        /// <returns><see cref="ClientSessionInfo"/></returns>
        public ClientSessionInfo? GetClientSessionForChannel(Socket? socket)
        {
            foreach (ClientSessionInfo clientSessionInfo in ClientSessions)
            {
                if (clientSessionInfo.ClientChannel?.Socket == socket)
                {
                    return clientSessionInfo;
                }
            }

            return null;
        }

        /// <summary>
        /// Checks if a message has been sent to the client.
        /// </summary>
        /// <param name="channel">The client channel to indicate the new msg sent</param>
        public void SetMsgSent(IChannel channel)
        {
            foreach(ClientSessionInfo clientSessionInfo in ClientSessions)
            {
                if(clientSessionInfo.ClientChannel == channel)
                {
                    clientSessionInfo.PingHandler.SentLocalMsg = true;
                }
            }
        }

        /// <summary>
        /// Removes the client session for channel.
        /// </summary>
        /// <param name="channel">he channel to remove the client session for</param>
        public void RemoveClientSessionForChannel(IChannel channel)
        {
            foreach(ClientSessionInfo clientSessionInfo in ClientSessions)
            {
                if(ReferenceEquals(clientSessionInfo.ClientChannel,channel))
                {
                    RemoveClientSession(clientSessionInfo);
                    break;
                }
            }
        }

        /// <summary>
        /// Removes the inactive client session for channel.
        /// </summary>
        /// <param name="clientSessionInfo">the client session info</param>
        public static void RemoveInactiveClientSessionForChannel(ClientSessionInfo clientSessionInfo)
        {
            clientSessionInfo.Clear();
        }

        /// <summary>
        ///  Allows the user to trace messages via XML.
        /// </summary>
        /// <param name="dictionary">dictionary for XML tracing</param>
        public void EnableXmlTrace(DataDictionary dictionary)
        {
            m_DictionaryForXml = dictionary;
            shouldXmlTrace = true;
        }

        /// <summary>
        /// Handles the ping processing for all active client sessions. Sends a ping
        /// to the client if the next send ping time has arrived and checks if a ping
        /// has been received from a client within the next receive ping time.
        /// </summary>
        public void HandlePings()
        {
            foreach (ClientSessionInfo clientSessionInfo in ClientSessions)
            {
                if (clientSessionInfo.ClientChannel != null && clientSessionInfo.ClientChannel.State == ChannelState.ACTIVE)
                {
                    TransportReturnCode ret = clientSessionInfo.PingHandler.HandlePings(clientSessionInfo.ClientChannel, out Error? error);
                    if (ret != TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine(error?.Text);
                        RemoveClientSession(clientSessionInfo);
                    }
                }
            }
        }

        private TransportReturnCode InitChannel(IChannel channel, out Error? error, IReceivedMsgCallback callback)
        {
            RemoveOption(channel, SelectMode.WRITE);

            m_InProgInfo.Clear();
            TransportReturnCode ret = channel.Init(m_InProgInfo, out error);
            if(ret < TransportReturnCode.SUCCESS)
            {
                Console.WriteLine($"sessionInactive: {error.Text}");
                RemoveClientSessionForChannel(channel);
                callback.ProcessChannelClose(channel);
                return TransportReturnCode.SUCCESS;
            }

            switch(ret)
            {
                case TransportReturnCode.CHAN_INIT_IN_PROGRESS:
                    if(m_InProgInfo.Flags == InProgFlags.SCKT_CHNL_CHANGE)
                    {
                        if(ReRegister(channel, m_InProgInfo, out error) < TransportReturnCode.SUCCESS)
                        {
                            return TransportReturnCode.FAILURE;
                        }
                        else
                        {
                            Console.WriteLine("Channel connection in progress");
                        }
                    }
                    break;
                case TransportReturnCode.SUCCESS:
                    Console.WriteLine("Client channel is now ACTIVE");
                    if(channel.Info(m_ChannelInfo, out error) == TransportReturnCode.SUCCESS)
                    {
                        Console.WriteLine($"Channel Info:\n" +
                            $" Connected Hostname: {m_ChannelInfo.ClientHostname}\n" +
                            $" Connected IP: {m_ChannelInfo.ClientIP}\n" +
                            $" Max Fragment Size: {m_ChannelInfo.MaxFragmentSize}\n" +
                            $" Output Buffers: {m_ChannelInfo.MaxOutputBuffers} Max, {m_ChannelInfo.GuaranteedOutputBuffers} Guaranteed\n" +
                            $" Input Buffers: {m_ChannelInfo.NumInputBuffers}\n" +
                            $" Send/Recv Buffer Sizes: {m_ChannelInfo.SysSendBufSize}/{m_ChannelInfo.SysRecvBufSize}\n" +
                            $" Ping Timeout: {m_ChannelInfo.PingTimeout}\n");
                        Console.WriteLine($" Client To Server Pings: {m_ChannelInfo.ClientToServerPings}\n Server to Client Pings: {m_ChannelInfo.ServerToClientPings}\n");
                        Console.WriteLine(" Connected component version: ");
                        if(m_ChannelInfo.ComponentInfoList is null || m_ChannelInfo.ComponentInfoList.Count == 0)
                        {
                            Console.WriteLine("No component info");
                        }
                        else
                        {
                            int count = m_ChannelInfo.ComponentInfoList.Count;
                            for(int i = 0; i < count; i++)
                            {
                                Console.WriteLine(m_ChannelInfo.ComponentInfoList[i].ComponentVersion);
                                if (i < count - 1)
                                    Console.WriteLine(", ");
                            }
                        }
                    }

                    Console.WriteLine("\n\n");

                    InitClientSession(channel);

                    break;

                default:
                    Console.WriteLine($"Bad return value ret={ret} {error.Text}");
                    break;
            }

            return TransportReturnCode.SUCCESS;
        }

        private void RemoveChannel(IChannel? channel)
        {
            if (channel is null) return;

            ClientSessionInfo? clientSessionInfo = GetClientSessionForChannel(channel);

            if (clientSessionInfo != null)
            {
                ClientSessionSocketList.Remove(clientSessionInfo.SelectElement);
                m_ClientChannelDict.Remove(channel.Socket);
                clientSessionInfo.SelectElement.Mode = SelectMode.NONE;
                clientSessionInfo.ClientChannel = null;
            }

            if (channel.State != ChannelState.INACTIVE)
            {
                TransportReturnCode ret = channel.Close(out Error error);
                if (ret < TransportReturnCode.SUCCESS)
                {
                    Console.Write($"IChannel.Close() failed with return code: {ret} and text: {error.Text}");
                }
            }
        }

        private void SetMsgReceived(IChannel channel)
        {
            foreach (ClientSessionInfo clientSessionInfo in ClientSessions)
            {
                if (clientSessionInfo.ClientChannel == channel)
                {
                    clientSessionInfo.PingHandler.ReceivedRemoteMsg = true;
                    break;
                }
            }
        }

        private void InitClientSession(IChannel channel)
        {
            foreach (ClientSessionInfo clientSessionInfo in ClientSessions)
            {
                if (clientSessionInfo.ClientChannel == channel)
                {
                    clientSessionInfo.PingHandler.InitPingHandler(clientSessionInfo.ClientChannel.PingTimeOut);
                    break;
                }
            }
        }

        private void HandleSocketChange(IChannel channel)
        {
            Console.WriteLine($"Read() Channel Change - Old Channel: {channel.OldSocket.Handle} " +
                $" New Channel: {channel.Socket.Handle}");

            ClientSessionInfo? clientSessionInfo = null;

            /* Removes the old Socket */
            if (channel.OldSocket != null)
            {
                for(int index = 0; index < ClientSessionSocketList.Count; index++)
                {
                    if (ClientSessionSocketList[index].Socket == channel.OldSocket)
                    {
                        clientSessionInfo = (ClientSessionInfo?)ClientSessionSocketList[index].UserSpec;

                        ClientSessionSocketList.RemoveAt(index);
                        m_ClientChannelDict.Remove(ClientSessionSocketList[index].Socket!);
                        break;
                    }
                }
            }

            /* Adds the new Socket */
            if(channel.Socket != null && clientSessionInfo != null)
            {
                clientSessionInfo.SelectElement.Socket = channel.Socket;
                ClientSessionSocketList.Add(clientSessionInfo.SelectElement);
                m_ClientChannelDict[channel.Socket] = channel;
            }
        }

        private TransportReturnCode ReRegister(IChannel channel, InProgInfo inProg, out Error? error)
        {
            error = null;

            ClientSessionInfo? clientSessionInfo = GetClientSessionForChannel(channel);

            if(clientSessionInfo is null)
                return TransportReturnCode.FAILURE;

            /* Removes the old channel's socket */
            if (inProg.OldSocket != null)
            {
                if (clientSessionInfo.SelectElement.Socket == inProg.OldSocket)
                {
                    ClientSessionSocketList.Remove(clientSessionInfo.SelectElement);
                    m_ClientChannelDict.Remove(clientSessionInfo.SelectElement.Socket);
                    clientSessionInfo.SelectElement.Socket = null;
                }
            }
            else
            {
                return TransportReturnCode.FAILURE;
            }

            /* Adds the new channel's socket */
            if(inProg.NewSocket != null)
            {
                clientSessionInfo.SelectElement.Socket = inProg.NewSocket;
                clientSessionInfo.SelectElement.Mode = SelectMode.READ;
                ClientSessionSocketList.Add(clientSessionInfo.SelectElement);
                m_ClientChannelDict[clientSessionInfo.SelectElement.Socket] = clientSessionInfo.ClientChannel!;
            }
            else
            {
                return TransportReturnCode.FAILURE;
            }

            return TransportReturnCode.SUCCESS;
        }

        private TransportReturnCode RegForWriteNotification(IChannel channel)
        {
            AddOption(channel, SelectMode.WRITE);

            return TransportReturnCode.SUCCESS;
        }

        private void RemoveClientSession(ClientSessionInfo clientSessionInfo)
        {
            m_ClientSessionCount--;
            RemoveChannel(clientSessionInfo.ClientChannel);
        }

        private TransportReturnCode ReadLoop(IChannel channel, out Error? error, IReceivedMsgCallback callback)
        {
            m_ReadArgs.Clear();
            do
            {
                ITransportBuffer msgBuf = channel.Read(m_ReadArgs, out error);
                if (msgBuf != null)
                {
                    if (shouldXmlTrace)
                    {
                        xmlString.Clear();
                        xmlString.Append("\nRead message: ");
                        xmlTraceDump.DumpBuffer(channel, (int)channel.ProtocolType, msgBuf, m_DictionaryForXml, xmlString, out error);
                        Console.WriteLine(xmlString);
                    }
                    callback.ProcessReceivedMsg(channel, msgBuf);
                    SetMsgReceived(channel);
                }
                else
                {
                    switch (m_ReadArgs.ReadRetVal)
                    {
                        case TransportReturnCode.FAILURE:
                        case TransportReturnCode.INIT_NOT_INITIALIZED:

                            Console.WriteLine("channelInactive chnl="
                                    + channel.Socket.Handle + " < "
                                    + error.Text + ">"
                                    + ",  state = " + channel.State);

                            RemoveClientSessionForChannel(channel);
                            callback.ProcessChannelClose(channel);
                            return TransportReturnCode.FAILURE;
                        case TransportReturnCode.READ_FD_CHANGE:
                            HandleSocketChange(channel);
                            break;
                        case TransportReturnCode.READ_PING:
                            /* Update ping monitor for server message received */
                            SetMsgReceived(channel);
                            break;
                        default:
                            if (m_ReadArgs.ReadRetVal < 0 && m_ReadArgs.ReadRetVal != TransportReturnCode.READ_WOULD_BLOCK)
                            {
                                Console.WriteLine("Read error=" + error.Text + "<" + m_ReadArgs.ReadRetVal + ">");
                            }
                            break;
                    }
                }
            }
            while (m_ReadArgs.ReadRetVal > TransportReturnCode.SUCCESS);

            return TransportReturnCode.SUCCESS;
        }

        private void AddOption(IChannel channel, SelectMode mode)
        {
            ClientSessionInfo? clientSessionInfo = GetClientSessionForChannel(channel);

            if(clientSessionInfo != null)
            {
                clientSessionInfo.SelectElement.Mode |= mode;
            }
        }

        private void RemoveOption(IChannel channel, SelectMode mode)
        {
            ClientSessionInfo? clientSessionInfo = GetClientSessionForChannel(channel);

            if (clientSessionInfo != null)
            {
                clientSessionInfo.SelectElement.Mode &= ~mode;
            }
        }

        /// <summary>
        /// Gets a channel from a socket
        /// </summary>
        /// <param name="socket">The socket to get a channel from</param>
        /// <returns><see cref="IChannel"/></returns>
        public IChannel? GetClientChannel(Socket socket)
        {
            if (m_ClientChannelDict.TryGetValue(socket, out IChannel? clientChannel))
            {
                return clientChannel;
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Gets the <see cref="ReadArgs"/> for this provider session.
        /// </summary>
        public ReadArgs? GetReadArgs => m_ReadArgs;
    }
}
