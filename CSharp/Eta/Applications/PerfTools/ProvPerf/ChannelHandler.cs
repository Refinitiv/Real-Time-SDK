/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.PerfTools.Common;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using System.Collections.Concurrent;
using System.Net.Sockets;
using ProviderSession = Refinitiv.Eta.PerfTools.Common.ProviderSession;

namespace Refinitiv.Eta.PerfTools.ProvPerf
{
    /// <summary>
    /// Performs operations associated with setting up and using ETA Channels, 
    /// such as initializing channels, reading, flushing, and checking ping timeouts.
    /// </summary>
    public class ChannelHandler
    {
        public ConcurrentBag<ClientChannelInfo> ActiveChannelList { get; set; }          // List of channels that are active.
        public ConcurrentBag<ClientChannelInfo> InitializingChannelList { get; set; }    // List of initializing channels.
        public IProviderThread ProviderThread { get; set; }                     // Reference to application-specified data.

        private Dictionary<IChannel, Example.Common.SelectMode> _clientOperation = new Dictionary<IChannel, Example.Common.SelectMode>();
        private Dictionary<Socket, ClientChannelInfo> m_SocketChannelDictionary = new Dictionary<Socket, ClientChannelInfo>();

        long m_Divisor = 1;

        private Error m_Error;
        private ReadArgs m_ReadArgs;
        private InProgInfo m_InProgInfo;
        private WriteArgs m_WriteArgs;

        public ReaderWriterLockSlim ChannelLock { get; set; } = new ReaderWriterLockSlim();

        List<Socket> m_SocketWriteList = new List<Socket>();
        List<Socket> m_SocketReadList = new List<Socket>();

        /// <summary>
        /// Instantiates a new channel handler.
        /// </summary>
        /// <param name="providerThread">the provider thread</param>
        public ChannelHandler(IProviderThread providerThread)
        {
            ActiveChannelList = new ConcurrentBag<ClientChannelInfo>();
            InitializingChannelList = new ConcurrentBag<ClientChannelInfo>();
            m_Error = new Error();
            m_ReadArgs = new ReadArgs();
            m_WriteArgs = new WriteArgs();
            m_InProgInfo = new InProgInfo();
            ProviderThread = providerThread;

            if (ProviderPerfConfig.TicksPerSec > 1000)
            {
                m_Divisor = 1000000;
            }
        }

        /// <summary>
        /// Iterates over new accepted client channel list and processes them 
        /// by initializing them and setting them up for read/write.
        /// </summary>
        public void ProcessNewChannels()
        {
            foreach (var channelInfo in InitializingChannelList)
            {
                InitializeChannel(channelInfo, out m_Error);
                if (channelInfo.Channel!.State == ChannelState.ACTIVE && channelInfo.Channel.Socket != null)
                {
                    m_SocketChannelDictionary.Add(channelInfo.Channel.Socket, channelInfo);
                }
            }
        }

        /// <summary>
        /// Adds a connected or accepted channel to the ChannelHandler
        /// </summary>
        /// <param name="channel">connected or accepted channel</param>
        /// <param name="userSpec">user object associated with the channel</param>
        /// <param name="checkPings">flag to check pings or not</param>
        /// <returns>client channel information for the connected or accepted channel</returns>
        public ClientChannelInfo AddChannel(IChannel channel, ProviderSession userSpec, bool checkPings)
        {
            ClientChannelInfo channelInfo = new ClientChannelInfo();
            channelInfo.UserSpec = userSpec;
            userSpec.Init(channelInfo);
            channelInfo.CheckPings = checkPings;          
            _clientOperation.Add(channel, Example.Common.SelectMode.NONE);
            channelInfo.Channel = channel;
            channelInfo.NeedRead = false;
            channelInfo.ParentQueue = InitializingChannelList;
            InitializingChannelList.Add(channelInfo);            

            if (channel.State == ChannelState.ACTIVE)
            {
                if (channelInfo.Channel.Socket != null)
                {
                    m_SocketChannelDictionary.Add(channelInfo.Channel.Socket, channelInfo);
                }
                // Channel is active
                ProcessActiveChannel(channelInfo);
            }

            return channelInfo;
        }

        /// <summary>
        /// Write a buffer to a channel
        /// </summary>
        /// <param name="clientChannelInfo">client channel info to write buffer to</param>
        /// <param name="msgBuffer">buffer to write</param>
        /// <param name="writeFlags">write flags</param>
        /// <param name="error">Error, to be populated in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/> value or the number of bytes pending flush</returns>
        public TransportReturnCode WriteChannel(ClientChannelInfo clientChannelInfo, ITransportBuffer msgBuffer, WriteFlags writeFlags, out Error error)
        {
            m_WriteArgs.Clear();
            m_WriteArgs.Priority = WritePriorities.HIGH;          
            m_WriteArgs.Flags = writeFlags;

            // write buffer
            TransportReturnCode ret = clientChannelInfo.Channel!.Write(msgBuffer, m_WriteArgs, out error);

            while (ret == TransportReturnCode.WRITE_CALL_AGAIN)
            {
                ret = clientChannelInfo.Channel.Flush(out error);
                if (ret < TransportReturnCode.SUCCESS)
                {
                    return ret;
                }
                ret = clientChannelInfo.Channel.Write(msgBuffer, m_WriteArgs, out error);
            }

            if (ret >= TransportReturnCode.SUCCESS)
            {
                clientChannelInfo.SentMsg = true;
                return ret;
            }

            switch (ret)
            {
                case TransportReturnCode.WRITE_FLUSH_FAILED:
                    if (clientChannelInfo.Channel.State == ChannelState.ACTIVE)
                    {
                        //
                        // Channel is still open, but ETA write() tried to flush
                        // internally and failed. Return positive value so the
                        // caller knows there's bytes to flush.
                        //
                        RequestFlush(clientChannelInfo);
                        return TransportReturnCode.SUCCESS + 1;
                    }
                    return ret;
                default:
                    return ret;
            }
        }

        private TransportReturnCode ReadChannel(ClientChannelInfo channelInfo, double stopTimeNsec, out Error? error)
        {
            TransportReturnCode readReturn = ReadFromChannel(channelInfo, stopTimeNsec, out error);

            if (readReturn != TransportReturnCode.SUCCESS)
            {
                // in case of read failure on a specific
                // client channel failure,
                // interactive provider needs to continue
                // running.
                return readReturn;
            }
            else if (readReturn > TransportReturnCode.SUCCESS)
            {
                channelInfo.NeedRead = true;
            }
            else
            {
                channelInfo.NeedRead = false;
            }
            return readReturn;
        }

        private TransportReturnCode ReadFromChannel(ClientChannelInfo clientChannelInfo, double stopTime, out Error? error)
        {
            TransportReturnCode ret = TransportReturnCode.SUCCESS;
            bool channelClosed = false;
            int readCount = -1; // allow at least one read

            m_ReadArgs.Clear();

            // Read until eta read() indicates that no more bytes are available in the queue.
            do
            {
                double currentTime = ProviderThread.CurrentTime!();

                if (readCount % 10 == 0)
                {
                    if (currentTime >= stopTime)
                    {
                        error = null;
                        return ret;
                    }                     
                }

                ITransportBuffer msgBuffer = clientChannelInfo.Channel!.Read(m_ReadArgs, out error);
                ret = m_ReadArgs.ReadRetVal;
                if (msgBuffer != null)
                {
                    // Mark that we received data for ping timeout handling.
                    clientChannelInfo.ReceivedMsg = true;

                    // Received a Msg Buffer, call the application's processing
                    // method.
                    if (ProviderThread.ProcessMsg(this, clientChannelInfo, msgBuffer, out error) < PerfToolsReturnCode.SUCCESS)
                    {
                        CloseChannel(clientChannelInfo, error!);
                        channelClosed = true;
                        ret = TransportReturnCode.FAILURE;
                    }
                }
                ++readCount;
            } while (ret > TransportReturnCode.SUCCESS);

            if (channelClosed)
            {
                return TransportReturnCode.FAILURE;
            }
                
            switch (ret)
            {
                case TransportReturnCode.SUCCESS:
                case TransportReturnCode.READ_WOULD_BLOCK:
                    return TransportReturnCode.SUCCESS;
                case TransportReturnCode.READ_FD_CHANGE:
                    m_SocketChannelDictionary.Remove(clientChannelInfo.Channel.OldSocket);
                    m_SocketChannelDictionary[clientChannelInfo.Channel.Socket] = clientChannelInfo;
                    return TransportReturnCode.SUCCESS;
                case TransportReturnCode.READ_PING:
                    clientChannelInfo.ReceivedMsg = true; // Mark that we received data for ping timeout handling
                    return TransportReturnCode.SUCCESS;
                default:
                    CloseChannel(clientChannelInfo, error!);
                    return TransportReturnCode.FAILURE;
            }
        }

        /// <summary>
        /// Determines operations necessary to perform on active channels and performs them
        /// </summary>
        /// <param name="stopTime">the value that determines when the loop should be terminated</param>
        /// <param name="error"><see cref="Error"/> instance with error details in case of failure</param>
        public void ReadChannels(double stopTime, out Error? error)
        {
            double currentTime;
            error = null;

            // Loop on select(), looking for channels with available data, until
            // stopTime is reached.
            m_SocketWriteList.Clear();
            m_SocketReadList.Clear();
            try
            {
                foreach (ClientChannelInfo channelInfo in ActiveChannelList)
                {
                    if (channelInfo.Channel != null && channelInfo.Channel.State == ChannelState.ACTIVE && channelInfo.Channel.Socket.Connected)
                    {
                        if (channelInfo.NeedFlush)
                        {
                            m_SocketWriteList.Add(channelInfo.Channel.Socket);
                        }                     
                        if (channelInfo.NeedRead)
                        {
                            ReadChannel(channelInfo, stopTime, out error);
                        }
                        m_SocketReadList.Add(channelInfo.Channel.Socket);
                    }
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.Message);
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = e.Message
                };
                return;
            }

            if (m_SocketReadList.Count > 0 || m_SocketWriteList.Count > 0)
            {
                currentTime = ProviderThread.CurrentTime!();
                try
                {
                    double selectTime = (stopTime - currentTime) / m_Divisor;

                    Socket.Select(m_SocketReadList, m_SocketWriteList, null, selectTime > 0 ? (int)selectTime : 0);

                    TransportReturnCode ret;
                    foreach (var socket in m_SocketReadList)
                    {
                        ClientChannelInfo channelInfo = m_SocketChannelDictionary[socket];
                        if (ReadChannel(channelInfo, stopTime, out error) < TransportReturnCode.SUCCESS)
                            continue;
                    }

                    foreach (var socket in m_SocketWriteList)
                    {
                        ClientChannelInfo channelInfo = m_SocketChannelDictionary[socket];
                        // Check if we can flush data from the channel.
                        if ((ret = channelInfo.Channel!.Flush(out error)) < TransportReturnCode.SUCCESS)
                        {
                            CloseChannel(channelInfo, error);
                            continue;
                        }
                        else if (ret == TransportReturnCode.SUCCESS)
                        {
                            // ETA flush() returned 0 instead of a higher
                            // value, so there's no more data to flush.
                            FlushDone(channelInfo);
                        }
                    }
                }
                catch (Exception e)
                {
                    Console.Error.WriteLine(e.Message);
                    Environment.Exit((int)TransportReturnCode.FAILURE);
                }
            }
        }

        /// <summary>
        /// Try to initialize a single channel.
        /// </summary>
        /// <param name="clientChannelInfo">channel to initialize</param>
        /// <param name="error"><see cref="Error"/> instance with error details</param>
        private void InitializeChannel(ClientChannelInfo clientChannelInfo, out Error error)
        {
            FlushDone(clientChannelInfo);

            switch (clientChannelInfo.Channel!.Init(m_InProgInfo, out error))
            {
                case TransportReturnCode.CHAN_INIT_IN_PROGRESS:
                    if (m_InProgInfo.Flags == InProgFlags.SCKT_CHNL_CHANGE)
                    {
                        m_SocketChannelDictionary.Remove(m_InProgInfo.OldSocket);
                        m_SocketChannelDictionary[m_InProgInfo.NewSocket] = clientChannelInfo;
                        RequestFlush(clientChannelInfo);
                    }
                    break;
                case TransportReturnCode.SUCCESS:
                    if (clientChannelInfo.Channel.State == ChannelState.ACTIVE)
                    {
                        ProcessActiveChannel(clientChannelInfo);
                    }
                    break;
                default:
                    CloseChannel(clientChannelInfo, error);
                    break;
            }
            return;
        }

        /// <summary>
        /// Performs ping timeout checks for channels in this channel handler.
        /// </summary>
        public void CheckPings()
        {
            long currentTime = (long)ProviderThread.CurrentTime!();
            foreach (ClientChannelInfo channelInfo in ActiveChannelList)
            {
                if (!channelInfo.CheckPings)
                {
                    continue;
                }

                // handle sending pings to clients
                if (currentTime >= channelInfo.NextSendPingTime)
                {
                    if (channelInfo.SentMsg)
                    {
                        channelInfo.SentMsg = false;
                    }
                    else
                    {
                        // send ping to remote (connection)
                        TransportReturnCode ret = channelInfo.Channel!.Ping(out m_Error);
                        if (ret > TransportReturnCode.SUCCESS)
                        {
                            RequestFlush(channelInfo);
                        }
                        else if (ret < TransportReturnCode.SUCCESS)
                        {
                            CloseChannel(channelInfo, m_Error); // Remove client if sending the message failed 
                            continue;
                        }
                    }
                    // set time to send next ping to client
                    channelInfo.NextSendPingTime = currentTime + 1000 * channelInfo.Channel!.PingTimeOut / 3;
                }

                // handle receiving pings from client
                if (currentTime >= channelInfo.NextReceivePingTime)
                {
                    // check if server received message from client since last time
                    if (channelInfo.ReceivedMsg)
                    {
                        // reset flag for client message received
                        channelInfo.ReceivedMsg = false;

                        // set time server should receive next message/ping from client 
                        channelInfo.NextReceivePingTime = currentTime + channelInfo.Channel!.PingTimeOut * 1000;
                    }
                    else // lost contact with client
                    {
                        Console.WriteLine($"{DateTime.Now}: Ping timed out. ping time out: {(1000 * channelInfo.Channel!.PingTimeOut / 3)}");
                        Console.Error.WriteLine("Ping timed out.");
                        CloseChannel(channelInfo, m_Error);
                    }
                }
            }
        }

        /// <summary>
        /// Cleans up a ChannelHandler
        /// </summary>
        public void Cleanup()
        {
            foreach (ClientChannelInfo channelInfo in ActiveChannelList)
            {
                CloseChannel(channelInfo, m_Error!);
                if (m_Error is not null)
                {
                    Console.WriteLine($"Error while closing channel {channelInfo.Channel!.Socket.Handle}: {m_Error.Text}");
                }
            }
        }

        /// <summary>
        /// Closes and removes a channel from the ChannelHandler.
        /// </summary>
        /// <param name="clientChannelInfo">client channel to close</param>
        /// <param name="error">If channel becomes inactive because of an error, this object gives detailed information about the error.</param>
        public void CloseChannel(ClientChannelInfo clientChannelInfo, Error error)
        {
            ProviderThread.ProcessInactiveChannel(this, clientChannelInfo, error);
            Error closeError;
            clientChannelInfo.Channel!.Close(out closeError);
            if (closeError is not null)
            {
                Console.WriteLine($"Error closing channel: {closeError.Text}");
            }
            clientChannelInfo.ParentQueue!.TryTake(out clientChannelInfo!);
        }

        private void ProcessActiveChannel(ClientChannelInfo clientChannelInfo)
        {
            // Set the next send/receive ping times.
            long currentTime = (long)ProviderThread.CurrentTime!();

            clientChannelInfo.NextSendPingTime = currentTime + 1000 * clientChannelInfo.Channel!.PingTimeOut / 3;
            clientChannelInfo.NextReceivePingTime = currentTime + clientChannelInfo.Channel.PingTimeOut * 1000;

            PerfToolsReturnCode ret = ProviderThread.ProcessActiveChannel(clientChannelInfo, out m_Error!);
            if (ret < PerfToolsReturnCode.SUCCESS)
            {
                if (m_Error is not null)
                {
                    Console.WriteLine($"Processing channel {clientChannelInfo.Channel.Socket.Handle} failed: {m_Error.Text}");
                }
                CloseChannel(clientChannelInfo, m_Error!);
                return;
            }
            ClientChannelInfo? activeChnl = clientChannelInfo;
            InitializingChannelList.TryTake(out activeChnl);
            clientChannelInfo.ParentQueue = ActiveChannelList;
            ActiveChannelList.Add(clientChannelInfo);
        }

        /// <summary>
        /// Mark flush not needed for the channel.
        /// </summary>
        /// <param name="clientChannelInfo">the client channel info</param>
        private void FlushDone(ClientChannelInfo clientChannelInfo)
        {
            clientChannelInfo.NeedFlush = false;
        }

        /// <summary>
        /// Requests that the ChannelHandler begin calling eta flush() for a channel. 
        /// Used when a call to eta write() indicates there is still data to be written to the network.
        /// </summary>
        /// <param name="clientChannelInfo">the client channel info</param>
        public void RequestFlush(ClientChannelInfo clientChannelInfo)
        {
            clientChannelInfo.NeedFlush = true;
        }

    }
}
