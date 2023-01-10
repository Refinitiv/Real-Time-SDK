/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Transports;
using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;

namespace LSEG.Eta.PerfTools.TransportPerf
{
    /// <summary>
    /// Handles one transport thread.
    /// </summary>
    public class TransportThread
    {
        private const int LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
        public const int TEST_PROTOCOL_TYPE = 88;

        private long m_ThreadIndex;

        /// <summary>
        /// Gets time of first connection
        /// </summary>
        public long ConnectTime { get; private set; }

        /// <summary>
        /// Gets time of last connection
        /// </summary>
        public long DisconnectTime { get; private set; }

        /// <summary>
        /// Gets a list of open channels and maintains those channels.
        /// </summary>
        public TransportChannelHandler? ChannelHandler { get; private set; }

        /// <summary>
        /// Gets the random latency array
        /// </summary>
        public LatencyRandomArray LatencyRandomArray { get; private set; } = new LatencyRandomArray();

        /// <summary>
        /// Gets total messages sent.
        /// </summary>
        public CountStat MsgsSent { get; private set; } = new CountStat();

        /// <summary>
        /// Gets total bytes sent(counting any compression).
        /// </summary>
        public CountStat BytesSent { get; private set; } = new CountStat();

        /// <summary>
        /// Gets total messages received.
        /// </summary>
        public CountStat MsgsReceived { get; private set; } = new CountStat();

        /// <summary>
        /// Gets total bytes received.
        /// </summary>
        public CountStat BytesReceived { get; private set; } = new CountStat();

        /// <summary>
        /// Gets messages not sent for lack of output buffers.
        /// </summary>
        public CountStat OutOfBuffersCount { get; private set; } = new CountStat();

        /// <summary>
        /// Gets latency statistics (recorded by stats thread).
        /// </summary>
        public ValueStatistics LatencyStats { get; private set; } = new ValueStatistics();

        /// <summary>
        /// Gets latency log file stream.
        /// </summary>
        public StreamWriter? LatencyLogFile { get; private set; }

        /// <summary>
        /// Gets statistics file stream for recording.
        /// </summary>
        public StreamWriter StatsFile { get; private set; }

        private SessionHandler? m_SessionHandler;    /* Session handler for this thread. */

        private LatencyRandomArrayOptions m_RandomArrayOpts = new LatencyRandomArrayOptions();

        private IProcessMsg? m_ProcessMsg;

        private ConnectOptions m_Copts = new ConnectOptions();
        private long m_NsecPerTick;
        private ChannelInfo m_ChnlInfo = new ChannelInfo();

        private IShutdownCallback? m_ShutdownCallback; /* shutdown callback to main application */
        private volatile bool m_Shutdown;                  /* Signals thread to shutdown. */
        private volatile bool m_ShutdownAck;               /* Acknowledges thread is shutdown. */

        internal int CurrentTicks { get; set; }     /* Current position in ticks per second. */

        /// <summary>
        /// Instantiates a new transport thread.
        /// </summary>
        /// <param name="threadIndex">The thread index</param>
        /// <param name="sessionHandler">The session handler</param>
        /// <param name="processMsg">The process msg</param>
        /// <param name="shutdownCallback">The shutdown callback</param>
        /// <param name="error">The error</param>
        public TransportThread(int threadIndex, SessionHandler sessionHandler, IProcessMsg processMsg, IShutdownCallback shutdownCallback, out Error? error)
        {
            error = null;
            MsgsSent.Init();
            BytesSent.Init();
            MsgsReceived.Init();
            BytesReceived.Init();
            OutOfBuffersCount.Init();
            LatencyStats.Clear();

            ConnectTime = 0;
            DisconnectTime = 0;
            m_ThreadIndex = threadIndex;
            m_SessionHandler = sessionHandler;
            m_ProcessMsg = processMsg;
            m_ShutdownCallback = shutdownCallback;
            ChannelHandler = new TransportChannelHandler(m_ShutdownCallback, out error);

            /* create stats file writer for this thread */
            string folderPath = "";
            Random random = new Random();
            try
            {
                folderPath = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);

                string tempStatFileName = $"{folderPath}{TransportPerfConfig.DIR_SEPERATOR}{TransportThreadConfig.StatsFileName}" +
                    $"{m_ThreadIndex + 1}_{random.NextInt64()}.csv";

                StatsFile = new StreamWriter(tempStatFileName);
            }
            catch (Exception)
            {
                Console.WriteLine($"Error: Failed to open stats file '{0}'.\n",
                                  TransportThreadConfig.StatsFileName);
                Environment.Exit(-1);
            }

            StatsFile.WriteLine("UTC, Msgs sent, Bytes sent, Msgs received, Bytes received, Latency msgs received, Latency avg (usec), Latency std dev (usec), Latency max (usec), Latency min (usec), CPU usage (%), Memory (MB)");

            /* create latency log file writer for this thread */
            if (TransportThreadConfig.LogLatencyToFile)
            {
                try
                {
                    string tempStatFileName = $"{folderPath}{TransportPerfConfig.DIR_SEPERATOR}{TransportThreadConfig.LatencyLogFileName}" +
                    $"{m_ThreadIndex + 1}_{random.NextInt64()}.csv";

                    LatencyLogFile = new StreamWriter(tempStatFileName);
                }
                catch (Exception)
                {
                    Console.WriteLine("Error: Failed to open latency log file '{0}'.\n", TransportThreadConfig.LatencyLogFileName);
                    Environment.Exit(-1);
                }

                LatencyLogFile.WriteLine("Send Time, Receive Time, Latency(nsec)");
            }

            CurrentTicks = 0;
            m_NsecPerTick = 1000000000L / TransportThreadConfig.TicksPerSec;
            ChannelHandler.Init(this);

            if (TransportThreadConfig.LatencyMsgsPerSec > 0)
            {
                m_RandomArrayOpts.TotalMsgsPerSec = TransportThreadConfig.MsgsPerSec;
                m_RandomArrayOpts.LatencyMsgsPerSec = TransportThreadConfig.LatencyMsgsPerSec;
                m_RandomArrayOpts.TicksPerSec = TransportThreadConfig.TicksPerSec;
                m_RandomArrayOpts.ArrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;
                if (LatencyRandomArray.Create(m_RandomArrayOpts) != PerfToolsReturnCode.SUCCESS)
                {
                    Console.WriteLine("Unable to create LatencyRandomArray");
                    m_ShutdownCallback.Shutdown();
                    m_ShutdownAck = true;
                }
            }
        }

        /// <summary>
        /// Cleanup.
        /// </summary>
        /// <param name="error">The error</param>
        public void Cleanup(Error error)
        {
            ChannelHandler!.Cleanup(error);
            StatsFile.Close();
            if (TransportThreadConfig.LogLatencyToFile)
            {
                LatencyLogFile!.Close();
            }
        }

        /// <summary>
        /// Process inactive channel.
        /// </summary>
        /// <param name="channelHandler">the channel handler</param>
        /// <param name="channelInfo">the channel info</param>
        /// <param name="error">the error</param>
        public void ProcessInactiveChannel(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, Error? error)
        {
            SessionHandler handler = (SessionHandler)channelHandler.UserSpec!;
            TransportSession session = (TransportSession)channelInfo.UserSpec!;

            /* If the channel was active and this is a client, we won't attempt to reconnect,
             * so stop the test. */
            if (!m_Shutdown &&
                TransportPerfConfig.AppType == TransportPerfConfig.ApplicationType.CLIENT &&
                session.TimeActivated > 0)
            {
                m_Shutdown = true;
            }

            if (error is null || error.ErrorId == TransportReturnCode.SUCCESS || "".Equals(error.Text))
            {
                Console.WriteLine("Channel Closed.\n");
            }
            else
            {
                Console.WriteLine($"Channel Closed: {0}({1})\n", error.ErrorId, error.Text);
            }

            handler.HandlerLock.EnterWriteLock();
            handler.OpenChannelsCount -= 1;
            handler.HandlerLock.ExitWriteLock();

            /* Record last disconnection time. */
            DisconnectTime = (long)GetTime.GetNanoseconds();

            m_ShutdownAck = true;
        }

        /// <summary>
        /// Process msg.
        /// </summary>
        /// <param name="channelHandler">the channel handler</param>
        /// <param name="channelInfo">the channel info</param>
        /// <param name="msgBuffer">the msg buffer</param>
        /// <param name="error">the error</param>
        /// <returns><see cref="PerfToolsReturnCode"/></returns>
        public PerfToolsReturnCode ProcessMsg(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, ITransportBuffer msgBuffer, out Error? error)
        {
            return m_ProcessMsg!.ProcessMsg(channelHandler, channelInfo, msgBuffer, out error);
        }

        /// <summary>
        /// Process active channel.
        /// </summary>
        /// <param name="channelHandler">the channel handler</param>
        /// <param name="channelInfo">the channel info</param>
        /// <param name="error">the error</param>
        /// <returns><see cref="PerfToolsReturnCode"/></returns>
        public TransportReturnCode ProcessActiveChannel(TransportChannelHandler channelHandler, ClientChannelInfo channelInfo, out Error? error)
        {
            error = null;
            TransportSession session = (TransportSession)channelInfo.UserSpec!;
            TransportReturnCode ret;

            if (TransportPerfConfig.HighWaterMark > 0)
            {
                if (channelInfo.Channel!.IOCtl(IOCtlCode.HIGH_WATER_MARK, TransportPerfConfig.HighWaterMark, out error) != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine("Channel.IOCtl() of HIGH_WATER_MARK failed <{0}>\n", error.Text);
                    return TransportReturnCode.FAILURE;
                }
            }

            /* Record first connection time. */
            if (ConnectTime == 0)
                ConnectTime = (long)GetTime.GetNanoseconds();

            /* retrieve and print out channel information */
            if ((ret = channelInfo.Channel!.Info(m_ChnlInfo, out error)) != TransportReturnCode.SUCCESS)
            {
                Console.WriteLine("Channel.info() failed: {0} <{1}>\n", ret, error.Text);
                return ret;
            }
            Console.WriteLine("Channel active. " + m_ChnlInfo + "\n");

            if (TransportThreadConfig.TotalBuffersPerPack > 1 &&
                ((TransportThreadConfig.MsgSize + 8) * TransportThreadConfig.TotalBuffersPerPack) > m_ChnlInfo.MaxFragmentSize)
            {
                Console.WriteLine("Error(Channel {0}): MaxFragmentSize {1} is too small for packing buffer size {2}\n",
                        channelInfo.Channel.Socket, m_ChnlInfo.MaxFragmentSize,
                        ((TransportThreadConfig.MsgSize + 8) * TransportThreadConfig.TotalBuffersPerPack));
                Environment.Exit(-1);
            }

            session.TimeActivated = (long)GetTime.GetNanoseconds();

            return TransportReturnCode.SUCCESS;
        }

        public void Start()
        {
            Thread runThread = new Thread(new ThreadStart(Run));
            runThread.Start();
        }

        public void Run()
        {
            long currentTime, nextTickTime;

            /* handle client session initialization */
            HandleClientSessionInit();

            nextTickTime = (long)GetTime.GetNanoseconds();

            /* this is the main loop */
            while (!m_Shutdown)
            {
                TransportReturnCode ret;
                currentTime = (long)GetTime.GetNanoseconds();

                if (currentTime >= nextTickTime)
                {
                    /* We've reached the next tick. Send a burst of messages out */
                    nextTickTime += m_NsecPerTick;

                    if (!TransportPerfConfig.BusyRead) // not busy read, read channels and send messages
                    {
                        if (ChannelHandler!.ReadChannels(nextTickTime, out Error? error) < PerfToolsReturnCode.SUCCESS)
                        {
                            m_ShutdownCallback!.Shutdown();
                            break;
                        }

                        //ClientChannelInfo clientChannelInfo;
                        /* send messages for writer role (reflector role doesn't send messages) */
                        if ((m_SessionHandler!.Role & TransportTestRole.WRITER) > 0)
                        {
                            foreach (var clientChannelInfo in ChannelHandler!.ActiveChannelList.Values)
                            {
                                TransportSession session;
                                session = (TransportSession)clientChannelInfo.UserSpec!;

                                /* The application corrects for ticks that don't finish before the time 
                                 * that the next message burst should start.  But don't do this correction 
                                 * for new channels. */
                                if (nextTickTime < session.TimeActivated)
                                {
                                    continue;
                                }

                                /* Send burst of messages */
                                ret = session.SendMsgBurst(this, out error);
                                ++CurrentTicks;
                                if (CurrentTicks == TransportThreadConfig.TicksPerSec)
                                    CurrentTicks = 0;
                                if (ret < TransportReturnCode.SUCCESS)
                                {
                                    switch (ret)
                                    {
                                        case TransportReturnCode.NO_BUFFERS:
                                            TransportChannelHandler.RequestFlush(clientChannelInfo);
                                            // if channel no longer active, close channel
                                            if (clientChannelInfo!.Channel!.State != ChannelState.ACTIVE)
                                                ChannelHandler.CloseChannel(clientChannelInfo, error);
                                            break;
                                        case TransportReturnCode.FAILURE:
                                            Console.WriteLine("Failure while writing message bursts: {0} ({1})\n", error?.Text, ret);
                                            ChannelHandler.CloseChannel(clientChannelInfo, error);
                                            break;
                                        default:
                                            Console.WriteLine("Failure while writing message bursts: {0} ({1})\n", error?.Text, ret);
                                            // if channel no longer active, close channel
                                            if (clientChannelInfo!.Channel!.State != ChannelState.ACTIVE)
                                                ChannelHandler.CloseChannel(clientChannelInfo, error);
                                            break;
                                    }
                                }
                                else if (ret > TransportReturnCode.SUCCESS)
                                {
                                    /* Need to flush */
                                    TransportChannelHandler.RequestFlush(clientChannelInfo);
                                }
                            }
                        }
                    }
                    else // busy read, messages not sent in this mode
                    {
                        //ClientChannelInfo clientChannelInfo;
                        foreach (var clientChannelInfo in ChannelHandler!.ActiveChannelList.Values)
                        {
                            //clientChannelInfo = ChannelHandler.ActiveChannelList[i];
                            if (!ChannelHandler.ReadChannel(clientChannelInfo, nextTickTime, out Error? error))
                            {
                                m_ShutdownCallback!.Shutdown();
                                break;
                            }
                        }

                        foreach (var clientChannelInfo in ChannelHandler!.InitializingChannelList.Values)
                        {
                            //clientChannelInfo = ChannelHandler.InitializingChannelList[i];
                            if (ChannelHandler.InitializeChannel(clientChannelInfo, out Error? error) < TransportReturnCode.SUCCESS)
                            {
                                m_ShutdownCallback!.Shutdown();
                                break;
                            }
                        }
                    }

                    /* handle server accepted session initialization */
                    HandleAcceptedSessionInit();

                    ret = ChannelHandler.CheckPings();
                    if (ret < TransportReturnCode.SUCCESS)
                    {
                        Error? error = null;
                        //ClientChannelInfo clientChannelInfo;
                        foreach (var clientChannelInfo in ChannelHandler!.ActiveChannelList.Values)
                        {
                            ChannelHandler.CloseChannel(clientChannelInfo, error);
                        }
                    }
                }
            }

            if (DisconnectTime == 0)
            {
                DisconnectTime = (long)GetTime.GetNanoseconds();
            }
        }

        private void HandleAcceptedSessionInit()
        {
            if (TransportPerfConfig.AppType == TransportPerfConfig.ApplicationType.SERVER)
            {
                /* Check if there are any new connections. */
                m_SessionHandler!.HandlerLock.EnterWriteLock();
                for (int i = 0; i < m_SessionHandler.NewChannelsList.Count; i++)
                {
                    new TransportSession(ChannelHandler!.AddChannel(m_SessionHandler.NewChannelsList[i], TransportThreadConfig.CheckPings));
                    m_SessionHandler.NewChannelsList.RemoveAt(i);
                }
                m_SessionHandler!.HandlerLock.ExitWriteLock();
            }
        }

        private void HandleClientSessionInit()
        {
            if (TransportPerfConfig.AppType == TransportPerfConfig.ApplicationType.CLIENT)
            {
                do
                {
                    TransportReturnCode ret;
                    IChannel? channel;
                    TransportSession session;

                    if ((channel = StartConnection()) == null)
                    {
                        Thread.Sleep(1000);
                        continue;
                    }

                    session = new TransportSession(ChannelHandler!.AddChannel(channel, TransportThreadConfig.CheckPings));

                    if (session.ChannelInfo!.Channel!.State != ChannelState.ACTIVE)
                    {
                        do
                        {
                            ret = ChannelHandler.WaitForChannelInit(session.ChannelInfo, 100000, out Error? error);
                        } while (!m_Shutdown && ret == TransportReturnCode.CHAN_INIT_IN_PROGRESS);

                        if (ret < TransportReturnCode.SUCCESS)
                        {
                            Thread.Sleep(1000);
                            continue;
                        }
                        else
                            break; /* Successful initialization. */
                    }
                    else
                        break;  /* Successful initialization. */

                } while (!m_Shutdown);
            }
        }

        private IChannel? StartConnection()
        {
            IChannel chnl;
            m_Copts.Clear();

            m_Copts.GuaranteedOutputBuffers = TransportPerfConfig.GuaranteedOutputBuffers;
            m_Copts.SysSendBufSize = TransportPerfConfig.SendBufSize;
            m_Copts.SysRecvBufSize = TransportPerfConfig.RecvBufSize;
            m_Copts.MajorVersion = 0;
            m_Copts.MinorVersion = 0;
            m_Copts.ProtocolType = (ProtocolType)TEST_PROTOCOL_TYPE;
            m_Copts.ConnectionType = TransportPerfConfig.ConnectionType;
            m_Copts.TcpOpts.TcpNoDelay = TransportPerfConfig.TcpNoDelay;
            m_Copts.CompressionType = TransportPerfConfig.CompressionType;
            m_Copts.UnifiedNetworkInfo.Address = TransportPerfConfig.HostName;
            m_Copts.UnifiedNetworkInfo.ServiceName = TransportPerfConfig.PortNo;

            if ((chnl = Transport.Connect(m_Copts, out Error error)) == null)
            {
                Console.WriteLine("Transport.Connect() failed: {0}({1})\n", error.ErrorId, error.Text);
                return null;
            }

            return chnl;
        }

        /// <summary>
        /// Submit a time record.
        /// </summary>
        /// <param name="recordQueue">the record queue</param>
        /// <param name="startTime">the start time</param>
        /// <param name="endTime">the end time</param>
        /// <param name="ticks">the ticks</param>
        public static void TimeRecordSubmit(TimeRecordQueue recordQueue, long startTime, long endTime, long ticks)
        {
            if(recordQueue.Pools.TryDequeue(out TimeRecord? record) == false)
            {
                record = new TimeRecord();
            }

            record.Ticks = ticks;
            record.StartTime = startTime;
            record.EndTime = endTime;

            recordQueue.Records.Enqueue(record);
        }

        /// <summary>
        /// Signals thread to shutdown.
        /// </summary>
        /// <param name="value">The value to indicate whether to shutdown</param>
        public void Shutdown(bool value)
        {
            m_Shutdown = value;
        }

        /// <summary>
        /// Acknowledges thread is shutdown.
        /// </summary>
        /// <returns><c>true</c>, if successful; otherwise <c>false</c></returns>
        public bool ShutdownAck()
        {
            return m_ShutdownAck;
        }
    }
}
