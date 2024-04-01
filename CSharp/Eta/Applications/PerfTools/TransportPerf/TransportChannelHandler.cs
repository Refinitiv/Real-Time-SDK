/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Net.Sockets;
using LSEG.Eta.Example.Common;
using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Common;
using LSEG.Eta.Transports;
using SelectMode = LSEG.Eta.Example.Common.SelectMode;

namespace LSEG.Eta.PerfTools.TransportPerf
{
    /// <summary>
    /// Performs the associated with setting up and using ETA Transport Channels, such as
    /// initializing channels, reading, flushing, and checking ping timeouts.
    /// </summary>
    public class TransportChannelHandler
    {
        private ReadArgs m_ReadArgs = new ReadArgs();
        private InProgInfo m_InProgInfo = new InProgInfo();

        private bool m_AtLeastOneChannel;  // set when we have at least one channel to process.

        private IShutdownCallback m_ShutdownCallback; // shutdown callback to main application

        private bool m_IsShutdown;

        private Dictionary<Socket, SelectElement> m_SelectElementDictionary = new Dictionary<Socket, SelectElement>();

        private List<Socket> m_ReadList = new List<Socket>();
        private List<Socket> m_WriteList = new List<Socket>();

        /// <summary>
        /// Gets a list of channels that are active.
        /// </summary>
        public ConcurrentDictionary<Guid, ClientChannelInfo> ActiveChannelList { get; private set; } = new ();

        /// <summary>
        /// Gets a list of initializing channels.
        /// </summary>
        public ConcurrentDictionary<Guid, ClientChannelInfo> InitializingChannelList { get; private set; } = new ();

        /// <summary>
        /// Gets a provider thread for the channel handler.
        /// </summary>
        public TransportThread? TransportThread { get; private set; }

        /// <summary>
        /// Gets or sets a reference to application-specified data.
        /// </summary>
        public object? UserSpec { get; set; }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="shutdownCallback"></param>
        /// <param name="error"></param>
        public TransportChannelHandler(IShutdownCallback shutdownCallback, out Error? error)
        {
            error = null;
            m_ShutdownCallback = shutdownCallback;
        }

        /// <summary>
        /// Initialize channel handler for a provider thread.
        /// </summary>
        /// <param name="transportThread">transport thread</param>
        public void Init(TransportThread transportThread)
        {
            TransportThread = transportThread;
        }

        /// <summary>
        /// Cleans up a ChannelHandler.
        /// </summary>
        /// <param name="error">the error</param>
        public void Cleanup(Error error)
        {
            foreach (ClientChannelInfo channelInfo in ActiveChannelList.Values)
            {
                CloseChannel(channelInfo, error);
            }
        }

        internal void CloseChannel(ClientChannelInfo clientChannelInfo, Error? error)
        {
            TransportThread!.ProcessInactiveChannel(this, clientChannelInfo, error);

            clientChannelInfo.Channel!.Close(out error);
            clientChannelInfo.ParentQueue!.TryRemove(clientChannelInfo.ID, out _);
            m_SelectElementDictionary.Remove(clientChannelInfo.Channel!.Socket);

            // if there is no more channels, reset atLeastOneChannel variable.
            if (ActiveChannelList.Count == 0 && InitializingChannelList.Count == 0)
                m_AtLeastOneChannel = false;

            if (!m_AtLeastOneChannel)
                m_ShutdownCallback.Shutdown();
        }

        /// <summary>
        /// Adds a connected or accepted channel to the ChannelHandler.
        /// </summary>
        /// <param name="channel">The connected or accepted channel</param>
        /// <param name="checkPings">The flag to check pings or not</param>
        /// <returns>client channel information for the connected or accepted channel</returns>
        public ClientChannelInfo AddChannel(IChannel channel, bool checkPings)
        {
            ClientChannelInfo channelInfo = new ClientChannelInfo();
            channelInfo.CheckPings = checkPings;
            channelInfo.ParentQueue = InitializingChannelList;
            channelInfo.Channel = channel;
            channelInfo.NeedRead = false;
            channelInfo.UserSpec = new TransportSession(channelInfo);
            InitializingChannelList.TryAdd(channelInfo.ID, channelInfo);

            SelectElement selectElement = new()
            {
                Socket = channel.Socket,
                UserSpec = channelInfo,
                Mode = SelectMode.READ
            };

            channelInfo.SelectElement = selectElement;
            m_SelectElementDictionary[channel.Socket] = selectElement;

            if (channel.State == ChannelState.ACTIVE)
            {
                // Channel is active
                ProcessActiveChannel(channelInfo);
            }

            m_AtLeastOneChannel = true;

            return channelInfo;

        }

        public bool ReadChannel(ClientChannelInfo channelInfo, double stopTimeNsec, out Error? error)
        {
            error = null;
            if (!m_IsShutdown)
            {
                PerfToolsReturnCode readReturn = ReadFromChannel(channelInfo, stopTimeNsec, out error);

                if ((readReturn < PerfToolsReturnCode.SUCCESS && channelInfo.Channel!.State == ChannelState.ACTIVE))
                {
                    Console.WriteLine("ReadFromChannel() failed with return code {0} <{1}>, exiting...", readReturn, error?.Text);
                    return false;
                }
                else if (readReturn > PerfToolsReturnCode.SUCCESS)
                {
                    channelInfo.NeedRead = true;
                }
                else
                {
                    channelInfo.NeedRead = false;
                }
            }

            return true;
        }

        private PerfToolsReturnCode ReadFromChannel(ClientChannelInfo clientChannelInfo, double stopTimeNsec, out Error? error)
        {
            error = null;
            TransportReturnCode ret = (TransportReturnCode)1;
            int readCount = -1; // allow at least one read

            m_ReadArgs.Clear();

            // Read until eta read() indicates that no more bytes are available in
            // the queue.
            do
            {
                if (readCount % 10 == 0)
                {
                    if (GetTime.GetNanoseconds() >= stopTimeNsec)
                        return (PerfToolsReturnCode)ret;
                }

                ITransportBuffer msgBuffer = clientChannelInfo.Channel!.Read(m_ReadArgs, out error);
                ret = m_ReadArgs.ReadRetVal;
                if (msgBuffer != null)
                {
                    // Mark that we received data for ping timeout handling.
                    clientChannelInfo.ReceivedMsg = true;

                    // Received a Msg Buffer, call the application's processing
                    // method.
                    if (TransportThread!.ProcessMsg(this, clientChannelInfo, msgBuffer, out error) < PerfToolsReturnCode.SUCCESS)
                    {
                        ret = TransportReturnCode.FAILURE;
                        break;
                    }
                }
                ++readCount;
            }
            while (ret > (int)TransportReturnCode.SUCCESS);

            switch (ret)
            {
                case TransportReturnCode.SUCCESS:
                case TransportReturnCode.READ_WOULD_BLOCK:
                case TransportReturnCode.READ_FD_CHANGE:
                    return PerfToolsReturnCode.SUCCESS;
                case TransportReturnCode.READ_PING:
                    clientChannelInfo.ReceivedMsg = true; // Mark that we received
                                                          // data for ping timeout
                                                          // handling.
                    return PerfToolsReturnCode.SUCCESS;
                default:
                    return PerfToolsReturnCode.FAILURE;
            }
        }

        public PerfToolsReturnCode ReadChannels(long stopTimeNsec, out Error? error)
        {
            error = null;
            long currentTime;

            // Loop on select(), looking for channels with available data, until
            // stopTime is reached.
            // Add active channels to the descriptor sets.
            try
            {
                foreach (ClientChannelInfo channelInfo in ActiveChannelList.Values)
                {
                    if (channelInfo.NeedFlush)
                    {
                        channelInfo.SelectElement!.Mode |= SelectMode.WRITE;
                    }
                    else
                    {
                        channelInfo.SelectElement!.Mode &= ~SelectMode.WRITE;
                    }

                    if (channelInfo.NeedRead)
                    {
                        if (!ReadChannel(channelInfo, stopTimeNsec, out error))
                            return PerfToolsReturnCode.FAILURE;
                    }
                }

                // Add initializing channels to the descriptor sets.
                foreach (ClientChannelInfo channelInfo in InitializingChannelList.Values)
                {
                    if (channelInfo.NeedFlush)
                    {
                        channelInfo.SelectElement!.Mode |= SelectMode.WRITE;
                    }
                    else
                    {
                        channelInfo.SelectElement!.Mode &= ~SelectMode.WRITE;
                    }

                    if (channelInfo.NeedRead)
                    {
                        if (!ReadChannel(channelInfo, stopTimeNsec, out error))
                            return PerfToolsReturnCode.FAILURE;
                    }
                }

                do
                {
                    m_ReadList.Clear();
                    m_WriteList.Clear();
                    Socket? socket;

                    foreach(var selectElement in m_SelectElementDictionary.Values)
                    {
                        socket = selectElement.Socket;
                        m_ReadList.Add(socket!);

                        if ((selectElement.Mode & SelectMode.WRITE) != 0)
                        {
                            m_WriteList.Add(socket!);
                        }
                    }

                    currentTime = (long)GetTime.GetNanoseconds();
                    try
                    {
                        if (m_ReadList.Count != 0 || m_WriteList.Count != 0)
                        {
                            long selTime = m_AtLeastOneChannel ? ((stopTimeNsec - currentTime) / 1000) : 1L;
                            if (selTime <= 0)
                                Socket.Select(m_ReadList, m_WriteList, null, 0);
                            else
                                Socket.Select(m_ReadList, m_WriteList, null, (int)selTime);
                        }
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine(e.Message);
                        return PerfToolsReturnCode.FAILURE;
                    }

                    if (m_ReadList.Count == 0 && m_WriteList.Count == 0 ) // timeout
                    {
                        return PerfToolsReturnCode.SUCCESS;
                    }
                    else // something to read or write
                    {
                        // Checks writeable sockets
                        for (int i = 0; i < m_WriteList.Count; i++)
                        {
                            socket = m_WriteList[i];

                            ClientChannelInfo? channelInfo = (ClientChannelInfo)m_SelectElementDictionary[socket].UserSpec!;

                            if (channelInfo.ParentQueue == ActiveChannelList)
                            {
                                TransportReturnCode ret;
                                // Check if we can flush data from the channel.
                                if ((ret = channelInfo.Channel!.Flush(out error)) < TransportReturnCode.SUCCESS)
                                {
                                    continue;
                                }
                                else if (ret == TransportReturnCode.SUCCESS)
                                {
                                    // ETA flush() returned 0 instead of a higher
                                    // value, so there's no more data to flush.
                                    FlushDone(channelInfo);
                                }
                            }
                            else if (channelInfo.ParentQueue == InitializingChannelList)
                            {
                                FlushDone(channelInfo);
                                InitializeChannel(channelInfo, out error);
                            }
                        }

                        // Checks for readable sockets
                        for (int i = 0; i < m_ReadList.Count; i++)
                        {
                            socket = m_ReadList[i];

                            ClientChannelInfo? channelInfo = (ClientChannelInfo)m_SelectElementDictionary[socket].UserSpec!;

                            if (channelInfo.ParentQueue == ActiveChannelList)
                            {
                                if (!ReadChannel(channelInfo, stopTimeNsec, out error))
                                    return PerfToolsReturnCode.FAILURE;
                            }
                            else if (channelInfo.ParentQueue == InitializingChannelList)
                            {
                                FlushDone(channelInfo);
                                InitializeChannel(channelInfo, out error);
                            }
                        }
                    }
                }
                while (currentTime < stopTimeNsec);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                return PerfToolsReturnCode.FAILURE;
            }

            return PerfToolsReturnCode.SUCCESS;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public TransportReturnCode CheckPings()
        {
            if (!m_IsShutdown)
            {
                double currentTime = GetTime.GetMilliseconds();
                foreach (ClientChannelInfo channelInfo in ActiveChannelList.Values)
                {
                    if (!channelInfo.CheckPings)
                        continue;

                    // handle sending pings to clients
                    if (currentTime >= channelInfo.NextSendPingTime)
                    {
                        if (channelInfo.SentMsg)
                        {
                            channelInfo.SentMsg = false;
                        }
                        else
                        {
                            /* send ping to remote (connection) */
                            TransportReturnCode ret = channelInfo.Channel!.Ping(out Error error);
                            if (ret > TransportReturnCode.SUCCESS)
                            {
                                RequestFlush(channelInfo);
                            }
                            else if (ret < TransportReturnCode.SUCCESS)
                            {
                                if (error.ErrorId == TransportReturnCode.FAILURE)
                                {
                                    Console.WriteLine(error.Text);
                                    return TransportReturnCode.FAILURE;
                                }
                            }
                        }

                        // set time to send next ping to client
                        channelInfo.NextSendPingTime = (long)currentTime + (channelInfo.Channel!.PingTimeOut / 3) * 1000;
                    }

                    // handle receiving pings from client
                    if (currentTime >= channelInfo.NextReceivePingTime)
                    {
                        // check if server received message from client since last time
                        if (channelInfo.ReceivedMsg)
                        {
                            // reset flag for client message received
                            channelInfo.ReceivedMsg = false;

                            // set time server should receive next message/ping from
                            // client
                            channelInfo.NextReceivePingTime = (long)currentTime + channelInfo.Channel!.PingTimeOut * 1000;
                        }
                        else
                        // lost contact with client
                        {
                            Console.WriteLine(DateTime.Now + ": Ping timed out. ping time out:" + (channelInfo.Channel!.PingTimeOut / 3) * 1000);
                            return TransportReturnCode.FAILURE;
                        }
                    }
                }
            }
            return TransportReturnCode.SUCCESS;
        }

        private static void FlushDone(ClientChannelInfo clientChannelInfo)
        {
            clientChannelInfo.NeedFlush = false;
        }

        public TransportReturnCode InitializeChannel(ClientChannelInfo clientChannelInfo, out Error error)
        {
            TransportReturnCode ret = TransportReturnCode.SUCCESS;
            FlushDone(clientChannelInfo);

            IChannel? channel = clientChannelInfo.Channel;

            switch (channel!.Init(m_InProgInfo, out error))
            {
                case TransportReturnCode.CHAN_INIT_IN_PROGRESS:
                    if (m_InProgInfo.Flags == InProgFlags.SCKT_CHNL_CHANGE)
                        RequestFlush(clientChannelInfo);
                    ret = TransportReturnCode.CHAN_INIT_IN_PROGRESS;
                    break;
                case TransportReturnCode.SUCCESS:
                    ProcessActiveChannel(clientChannelInfo);
                    break;
                default:
                    ret = TransportReturnCode.FAILURE;
                    m_IsShutdown = true;
                    m_ShutdownCallback.Shutdown();
                    break;
            }
            return ret;
        }

        public TransportReturnCode WaitForChannelInit(ClientChannelInfo clientChannelInfo, int waitTimeMilliSec, out Error? error)
        {
            error = null;
            //
            // NeedFlush indicates if we want to call eta InitChannel()
            // immediately.
            //
            if (clientChannelInfo.NeedFlush)
            {
                clientChannelInfo.SelectElement!.Mode = (SelectMode.READ | SelectMode.WRITE);
            }
            else
            {
                clientChannelInfo.SelectElement!.Mode = SelectMode.READ;
            }

            if(clientChannelInfo.Channel!.Socket.Poll(0, System.Net.Sockets.SelectMode.SelectRead) || 
                clientChannelInfo.Channel!.Socket.Poll(0, System.Net.Sockets.SelectMode.SelectWrite))
            {
                return InitializeChannel(clientChannelInfo, out error);
            }
            else
            {
                return TransportReturnCode.CHAN_INIT_IN_PROGRESS;
            }
        }

        /// <summary>
        /// Requests that the ChannelHandler begin calling ETA Flush() for a channel.
        /// Used when a call to ETA Write() indicates there is still data to be
        /// written to the network.
        /// </summary>
        /// <param name="clientChannelInfo">the client channel info</param>
        public static void RequestFlush(ClientChannelInfo clientChannelInfo)
        {
            clientChannelInfo.NeedFlush = true;
        }

        private void ProcessActiveChannel(ClientChannelInfo clientChannelInfo)
        {
            // Set the next send/receive ping times.
            double currentTime = GetTime.GetMilliseconds();

            clientChannelInfo.NextSendPingTime = (long)currentTime + clientChannelInfo.Channel!.PingTimeOut / 3 * 1000;
            clientChannelInfo.NextReceivePingTime = (long)currentTime + clientChannelInfo.Channel!.PingTimeOut * 1000;

            FlushDone(clientChannelInfo);

            PerfToolsReturnCode ret = (PerfToolsReturnCode)TransportThread!.ProcessActiveChannel(this, clientChannelInfo, out Error? error);
            if (ret < PerfToolsReturnCode.SUCCESS)
            {
                return;
            }

            if (InitializingChannelList.TryRemove(clientChannelInfo.ID, out _))
            {
                clientChannelInfo.ParentQueue = ActiveChannelList;
                clientChannelInfo.ParentQueue.TryAdd(clientChannelInfo.ID, clientChannelInfo);
            }
        }
    }
}
