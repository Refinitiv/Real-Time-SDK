/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Transports;
using System;
using System.Diagnostics;
using System.IO;
using System.Threading;

namespace LSEG.Eta.PerfTools.TransportPerf
{
    /// <summary>
    /// The TransportPerf application. This application may act as a clienl or server as appropriate,
    /// and tests the sending of raw messages across connections.
    /// <para>
    /// The purpose of this application is to measure performance of the ETA transport,
    /// using the different supported connection types with variable message sizes.
    /// </para>
    /// <para>
    /// The content provided by this application is intended for raw transport measurement and
    /// does not use the OMM encoders and decoders. The message contains only a sequence number and random
    /// timestamp. The remainder of the message is padded with zeros.
    /// </para>
    /// <para>
    /// The application creates two types of threads:
    /// A main thread, which collects and records statistical information.
    /// Connection threads, each of which connect or accept connections, and pass messages across.
    /// </para>
    /// <para>
    /// To measure latency, a timestamp is randomly placed in each burst of messages sent.
    /// The receiver of the messages reads the timestamp and compares it to the current time to determine
    /// the end-to-end latency. 
    /// </para>
    /// <para>
    /// For more detailed information on the performance measurement applications,
    /// see the ETA C# Open Source Performance Tools Guide.
    /// </para>
    /// </summary>
    public class TransportPerf : IShutdownCallback
    {
        private int m_SessionHandlerCount;
        private SessionHandler[]? m_SessionHandlerList;
        private IProcessMsg? m_ProcessMsg;
        private InitArgs m_InitArgs; /* arguments for initializing transport */
        private bool m_RunTimeExpired; /* application run-time expired */
        private IServer? m_Server; // used if we do a Transport.bind
        private AcceptOptions m_AcceptOptions; // server accept options

        /* CPU & Memory Usage samples */
        private ValueStatistics m_CpuUsageStats, m_MemUsageStats;

        private ValueStatistics m_TotalLatencyStats, m_IntervalLatencyStats;

        private long m_TotalMsgSentCount = 0;
        private long m_TotalBytesSent = 0;
        private long m_TotalMsgReceivedCount = 0;
        private long m_TotalBytesReceived = 0;

        /* Logs summary information, such as application inputs and final statistics. */
        private StreamWriter? m_SummaryFileWriter;
        private StreamWriter m_StdOutWriter; /* stdio writer */

        private bool m_NegativeLatencyFound; // negative latency found flag

        private Error? m_Error;

        /* indicates whether or not application should be shutdown */
        private static volatile bool ShutDownApp = false;

        /* Gets resource usage */
        private ResourceUsageStats m_ResourceUsageStats = new ResourceUsageStats();

        public TransportPerf()
        {
            m_InitArgs = new InitArgs();
            m_CpuUsageStats = new ValueStatistics();
            m_MemUsageStats = new ValueStatistics();
            m_TotalLatencyStats = new ValueStatistics();
            m_IntervalLatencyStats = new ValueStatistics();
            m_AcceptOptions = new AcceptOptions();
            m_StdOutWriter = new StreamWriter(Console.OpenStandardOutput());
        }

        /// <summary>
        /// Shutdown TransportPerf
        /// </summary>
        public void Shutdown()
        {
            ShutDownApp = true;
        }

        /* Initializes TransportPerf application. */
        private void Initialize(string[] args)
        {
            TransportPerfConfig.Init(args);
            Console.WriteLine(TransportPerfConfig.ConfigString());

            if (TransportPerfConfig.AppType == TransportPerfConfig.ApplicationType.SERVER &&
                TransportPerfConfig.ConnectionType == ConnectionType.ENCRYPTED)
            {
                Console.WriteLine("Error: Does not support encrypted connectionType while running as server.");
                Environment.Exit(-1);
            }

            /* Create summary file writer */
            try
            {
                m_SummaryFileWriter = new StreamWriter(TransportPerfConfig.SummaryFileName!);
            }
            catch (Exception exp)
            {
                Console.WriteLine($"Error: Failed to open summary file '{TransportPerfConfig.SummaryFileName}', Text = {exp.Message}.\n");
                Environment.Exit(-1);
            }

            /* Write configuration parameters to summary file */
            m_SummaryFileWriter.WriteLine(TransportPerfConfig.ConfigString());
            m_SummaryFileWriter.Flush();

            m_SessionHandlerCount = TransportPerfConfig.ThreadCount;
            m_SessionHandlerList = new SessionHandler[m_SessionHandlerCount];
            for (int i = 0; i < m_SessionHandlerCount; i++)
            {
                m_SessionHandlerList[i] = new SessionHandler();
            }

            /* Initialize ETA */
            m_InitArgs.Clear();
            m_InitArgs.GlobalLocking = TransportPerfConfig.ThreadCount > 1;
            if (Transport.Initialize(m_InitArgs, out Error m_Error) != TransportReturnCode.SUCCESS)
            {
                Console.WriteLine($"Error: Transport failed to initialize: {m_Error.Text}");
                Environment.Exit(-1);
            }

            /* Spawn threads */
            for (int i = 0; i < m_SessionHandlerCount; ++i)
            {
                m_SessionHandlerList[i].Init();
                if (TransportPerfConfig.ReflectMsgs)
                {
                    m_SessionHandlerList[i].Role = TransportTestRole.REFLECTOR;
                    m_ProcessMsg = new ProcessMsgReflect();
                }
                else
                {
                    m_SessionHandlerList[i].Role = (TransportTestRole.WRITER | TransportTestRole.READER);
                    m_ProcessMsg = new ProcessMsgReadWrite();
                }
                m_SessionHandlerList[i].Active = true;
                m_SessionHandlerList[i].TransportThread = new TransportThread(i, m_SessionHandlerList[i], m_ProcessMsg, this, out m_Error!);
                m_SessionHandlerList[i].TransportThread!.ChannelHandler!.UserSpec = (m_SessionHandlerList[i]);
                m_SessionHandlerList[i].TransportThread!.Start();
            }

            /* Bind server if server mode */
            if (TransportPerfConfig.AppType == TransportPerfConfig.ApplicationType.SERVER)
            {
                m_AcceptOptions.SysSendBufSize = TransportPerfConfig.SendBufSize;
                m_AcceptOptions.SysRecvBufSize = TransportPerfConfig.RecvBufSize;
                if ((m_Server = BindServer(out m_Error)) == null)
                {
                    Console.WriteLine($"Bind failed: {m_Error.Text}");
                    Environment.Exit(-1);
                }
            }

            m_CpuUsageStats.Clear();
            m_MemUsageStats.Clear();
            m_TotalLatencyStats.Clear();

            m_ResourceUsageStats.Init();
        }

        private void Run(String[] args)
        {
            // This loop will block in selector for up to 1 second and wait for accept
            // If any channel is trying to connect during this time it will be accepted
            // the time tracking parameters and counters are updated at the second interval 
            // at the configured time intervals the stats will be printed
            // the loop exits when time reaches the configured end time.

            long nextTime = (long)GetTime.GetMilliseconds() + 1000L;
            int intervalSeconds = 0;
            int currentRuntimeSec = 0;
            int writeStatsInterval = TransportPerfConfig.WriteStatsInterval;
            bool displayStats = TransportPerfConfig.DisplayStats;

            // this is the main loop
            while (true)
            {
                bool selectRetVal;
                long selectTime = nextTime - (long)GetTime.GetMilliseconds();

                if (m_Server != null)
                {
                    if (selectTime > 0)
                        selectRetVal = m_Server!.Socket.Poll((int)selectTime * 1000, System.Net.Sockets.SelectMode.SelectRead);
                    else
                        selectRetVal = m_Server!.Socket.Poll(0, System.Net.Sockets.SelectMode.SelectRead);


                    if (selectRetVal)
                    {
                        IChannel clientChannel = m_Server.Accept(m_AcceptOptions, out m_Error);
                        if (clientChannel == null)
                        {
                            Console.Write("ETA Server Accept failed: <{0}>\n", m_Error.Text);
                        }
                        else
                        {
                            Console.Write("Server accepting new channel '{0}'.\n\n", clientChannel.Socket.Handle);
                            SendToLeastLoadedThread(clientChannel);
                        }
                    }
                }
                else
                {
                    if (selectTime > 0)
                    {
                        Thread.Sleep((int)selectTime);
                    }
                    else
                    {
                        Thread.Sleep(0);
                    }
                }

                if ((long)GetTime.GetMilliseconds() >= nextTime)
                {
                    nextTime += 1000L;
                    ++intervalSeconds;
                    ++currentRuntimeSec;

                    // Check if it's time to print stats
                    if (intervalSeconds >= writeStatsInterval)
                    {
                        CollectStats(true, displayStats, currentRuntimeSec, writeStatsInterval);
                        intervalSeconds = 0;
                    }
                }
                /* Handle run-time */
                HandleRuntime(currentRuntimeSec);
            }
        }

        private void CollectStats(bool writeStats, bool displayStats, int currentRuntimeSec, int timePassedSec)
        {
            long intervalMsgSentCount, intervalBytesSent,
                intervalMsgReceivedCount, intervalBytesReceived,
                intervalOutOfBuffersCount;
            m_ResourceUsageStats.Refresh();
            double currentProcessCpuLoad = m_ResourceUsageStats.CurrentProcessCpuLoad();
            double currentMemoryUsage = m_ResourceUsageStats.CurrentMemoryUsage();

            if (timePassedSec > 0)
            {
                m_CpuUsageStats.Update(currentProcessCpuLoad);
                m_MemUsageStats.Update(currentMemoryUsage);
            }

            for (int i = 0; i < m_SessionHandlerCount; ++i)
            {
                TimeRecordQueue latencyRecords = m_SessionHandlerList![i].LatencyRecords;
                m_IntervalLatencyStats.Clear();

                while (!latencyRecords.Records.IsEmpty)
                {
                    latencyRecords.Records.TryDequeue(out TimeRecord? record);
                    double latency = (record!.EndTime - record.StartTime) / (double)record.Ticks;
                    if (record.StartTime > record.EndTime)
                    {   // if the start time is after the end time, then there is probably an issue with timing on the machine
                        if (!m_NegativeLatencyFound)
                        {
                            Console.Write("Warning: negative latency calculated. The clocks do not appear to be synchronized. (start={0:D}, end={1:D})\n", record.StartTime, record.EndTime);
                            m_SummaryFileWriter!.Write("Warning: negative latency calculated. The clocks do not appear to be synchronized. (start={0:D}, end={1:D})\n", record.StartTime, record.EndTime);
                        }

                        m_NegativeLatencyFound = true;
                    }
                    m_IntervalLatencyStats.Update(latency);
                    m_SessionHandlerList[i].TransportThread!.LatencyStats.Update(latency);
                    m_TotalLatencyStats.Update(latency);

                    if (TransportThreadConfig.LogLatencyToFile)
                    {
                        m_SessionHandlerList[i].TransportThread!.LatencyLogFile!.Write("{0:D}, {1:D}, {2:D}\n", record.StartTime, record.EndTime, record.EndTime - record.StartTime);
                    }
                }

                if (TransportThreadConfig.LogLatencyToFile)
                {
                    m_SessionHandlerList[i].TransportThread!.LatencyLogFile!.Flush();
                }

                intervalMsgSentCount = m_SessionHandlerList[i].TransportThread!.MsgsSent.GetChange();
                intervalBytesSent = m_SessionHandlerList[i].TransportThread!.BytesSent.GetChange();
                intervalMsgReceivedCount = m_SessionHandlerList[i].TransportThread!.MsgsReceived.GetChange();
                intervalBytesReceived = m_SessionHandlerList[i].TransportThread!.BytesReceived.GetChange();
                intervalOutOfBuffersCount = m_SessionHandlerList[i].TransportThread!.OutOfBuffersCount.GetChange();

                m_TotalMsgSentCount += intervalMsgSentCount;
                m_TotalBytesSent += intervalBytesSent;
                m_TotalMsgReceivedCount += intervalMsgReceivedCount;
                m_TotalBytesReceived += intervalBytesReceived;

                if (writeStats)
                {
                    TransportThread thread = m_SessionHandlerList[i].TransportThread!;

                    PrintCurrentTimeUTC(thread.StatsFile);
                    thread.StatsFile.Write(", {0:D}, {1:D}, {2:D}, {3:D}, {4:D}, {5:F3}, {6:F3}, {7:F3}, {8:F3}, {9:F2}, {10:F2}\n",
                            intervalMsgSentCount,
                            intervalBytesSent,
                            intervalMsgReceivedCount,
                            intervalBytesReceived,
                            m_IntervalLatencyStats.Count,
                            m_IntervalLatencyStats.Average,
                            Math.Sqrt(m_IntervalLatencyStats.Variance),
                            m_IntervalLatencyStats.Count > 0 ? m_IntervalLatencyStats.MaxValue : 0.0,
                            m_IntervalLatencyStats.Count > 0 ? m_IntervalLatencyStats.MinValue : 0.0,
                            currentProcessCpuLoad,
                            currentMemoryUsage);
                    thread.StatsFile.Flush();
                }

                if (displayStats)
                {
                    if (TransportPerfConfig.ThreadCount == 1)
                        Console.Write("{0,3:D}:\n", currentRuntimeSec);
                else
                        Console.Write("{0,3:D}: Thread {1:D}:\n", currentRuntimeSec, i + 1);

                    Console.Write("  Sent: MsgRate: {0,8:F}, DataRate:{1,8:F3}MBps\n",
                            (double)intervalMsgSentCount / (double)timePassedSec,
                            (double)intervalBytesSent / (double)(1024 * 1024) / timePassedSec);

                    Console.Write("  Recv: MsgRate: {0,8:F}, DataRate:{1,8:F3}MBps\n",
                            (double)intervalMsgReceivedCount / (double)timePassedSec,
                            (double)intervalBytesReceived / (double)(1024 * 1024) / timePassedSec);

                    if (intervalOutOfBuffersCount > 0)
                    {
                        Console.Write("  {0:D} messages not sent due to lack of output buffers.\n", intervalOutOfBuffersCount);
                    }

                    if (m_IntervalLatencyStats.Count > 0)
                    {
                        m_IntervalLatencyStats.Print("  Latency(usec)", "Msgs", true);
                    }

                    Console.Write("  CPU: {0,6:F2}% Mem: {1,8:F2}MB\n", currentProcessCpuLoad, currentMemoryUsage);
                }
            }
        }

        private void HandleRuntime(int currentRunTime)
        {
            if (currentRunTime >= TransportPerfConfig.RunTime)
            {
                Console.Write("\nRun time of {0:D} seconds has expired.\n\n", TransportPerfConfig.RunTime);
                m_RunTimeExpired = true;
            }

            if (m_RunTimeExpired == true || ShutDownApp == true)
            {
                for (int i = 0; i < m_SessionHandlerCount; ++i)
                {
                    m_SessionHandlerList![i].TransportThread!.Shutdown(true);

                    int timeout = 0;

                    // wait for consumer thread cleanup or timeout
                    while (!m_SessionHandlerList[i].TransportThread!.ShutdownAck() && timeout < 1)
                    {
                        System.Threading.Thread.Sleep(1000);
                        timeout++;
                    }
                }
            }

            if (m_RunTimeExpired == true || ShutDownApp == true)
            {
                CleanUpAndExit();
            }
        }

        private void CleanUpAndExit()
        {
            Console.WriteLine("Shutting down.\n");

            /* if server, close it */
            if (TransportPerfConfig.AppType == TransportPerfConfig.ApplicationType.SERVER)
            {
                m_Server!.Close(out m_Error);
            }

            Transport.Uninitialize();

            CollectStats(false, false, 0, 0);
            PrintSummaryStats(m_StdOutWriter);
            m_StdOutWriter.Flush();
            PrintSummaryStats(m_SummaryFileWriter!);
            m_SummaryFileWriter!.Close();
            Environment.Exit(0);
        }

        private void PrintSummaryStats(StreamWriter fileWriter)
        {
            long earliestConnectTime, latestDisconnectTime;
            double connectedTime;

            earliestConnectTime = 0;
            latestDisconnectTime = 0;
            for (int i = 0; i < TransportPerfConfig.ThreadCount; ++i)
            {
                if (earliestConnectTime == 0 ||
                    m_SessionHandlerList![i].TransportThread!.ConnectTime < earliestConnectTime)
                {
                    earliestConnectTime = m_SessionHandlerList![i].TransportThread!.ConnectTime;
                }

                if (latestDisconnectTime == 0 ||
                    m_SessionHandlerList![i].TransportThread!.DisconnectTime > latestDisconnectTime)
                {
                    latestDisconnectTime = m_SessionHandlerList![i].TransportThread!.DisconnectTime;
                }
            }

            connectedTime = ((double)latestDisconnectTime - (double)earliestConnectTime) / 1000000000.0;

            if (TransportPerfConfig.ThreadCount > 1)
            {
                for (int i = 0; i < TransportPerfConfig.ThreadCount; ++i)
                {
                    TransportThread thread = m_SessionHandlerList![i].TransportThread!;

                    double threadConnectedTime = ((double)m_SessionHandlerList[i].TransportThread!.DisconnectTime
                            - (double)m_SessionHandlerList[i].TransportThread!.ConnectTime) / 1000000000.0;

                    fileWriter.Write("--- THREAD %d SUMMARY ---\n\n", i + 1);

                    fileWriter.Write("Statistics: \n");

                    if (thread.LatencyStats.Count > 0)
                    {
                        fileWriter.Write("  Latency avg (usec): {0:F3}\n", thread.LatencyStats.Average);
                        fileWriter.Write("  Latency std dev (usec): {0:F3}\n", Math.Sqrt(thread.LatencyStats.Variance));
                        fileWriter.Write("  Latency max (usec): {0:F3}\n", thread.LatencyStats.MaxValue);
                        fileWriter.Write("  Latency min (usec): {0:F3}\n", thread.LatencyStats.MinValue);
                    }
                    else
                        fileWriter.Write("  No latency information was received.\n\n");

                    fileWriter.Write("  Sampling duration(sec): {0:F2}\n", threadConnectedTime);
                    fileWriter.Write("  Msgs Sent: {0:D}\n", thread.MsgsSent.GetTotal());
                    fileWriter.Write("  Msgs Received: {0:D}\n", thread.MsgsReceived.GetTotal());
                    fileWriter.Write("  Data Sent (MB): {0:F2}\n", thread.BytesSent.GetTotal() / 1048576.0);
                    fileWriter.Write("  Data Received (MB): {0:F2}\n", thread.BytesReceived.GetTotal() / 1048576.0);
                    fileWriter.Write("  Avg. Msg Sent Rate: {0:F0}\n", threadConnectedTime > 0 ? thread.MsgsSent.GetTotal() / threadConnectedTime : 0);
                    fileWriter.Write("  Avg. Msg Recv Rate: {0:F0}\n", threadConnectedTime > 0 ? thread.MsgsReceived.GetTotal() / threadConnectedTime : 0);
                    fileWriter.Write("  Avg. Data Sent Rate (MB): {0:F2}\n", threadConnectedTime > 0 ? thread.BytesSent.GetTotal() / 1048576.0 / threadConnectedTime : 0);
                    fileWriter.Write("  Avg. Data Recv Rate (MB): {0:F2}\n\n", threadConnectedTime > 0 ? thread.BytesReceived.GetTotal() / 1048576.0 / threadConnectedTime : 0);
                }
            }

            fileWriter.Write("--- OVERALL SUMMARY ---\n\n");

            fileWriter.Write("Statistics: \n");

            if (m_TotalLatencyStats.Count > 0)
            {
                fileWriter.Write("  Latency avg (usec): {0:F3}\n", m_TotalLatencyStats.Average);
                fileWriter.Write("  Latency std dev (usec): {0:F3}\n", Math.Sqrt(m_TotalLatencyStats.Variance));
                fileWriter.Write("  Latency max (usec): {0:F3}\n", m_TotalLatencyStats.MaxValue);
                fileWriter.Write("  Latency min (usec): {0:F3}\n", m_TotalLatencyStats.MinValue);
            }
            else
                fileWriter.Write("  No latency information was received.\n\n");

            fileWriter.Write("  Sampling duration(sec): {0:F2}\n", connectedTime);
            fileWriter.Write("  Msgs Sent: {0:D}\n", m_TotalMsgSentCount);
            fileWriter.Write("  Msgs Received: {0:D}\n", m_TotalMsgReceivedCount);
            fileWriter.Write("  Data Sent (MB): {0:F2}\n", m_TotalBytesSent / 1048576.0);
            fileWriter.Write("  Data Received (MB): {0:F2}\n", m_TotalBytesReceived / 1048576.0);
            fileWriter.Write("  Avg. Msg Sent Rate: {0:F0}\n", connectedTime > 0 ? m_TotalMsgSentCount / connectedTime : 0);
            fileWriter.Write("  Avg. Msg Recv Rate: {0:F0}\n", connectedTime > 0 ? m_TotalMsgReceivedCount / connectedTime : 0);
            fileWriter.Write("  Avg. Data Sent Rate (MB): {0:F2}\n", connectedTime > 0 ? m_TotalBytesSent / 1048576.0 / connectedTime : 0);
            fileWriter.Write("  Avg. Data Recv Rate (MB): {0:F2}\n", connectedTime > 0 ? m_TotalBytesReceived / 1048576.0 / connectedTime : 0);

            if (m_CpuUsageStats.Count > 0)
            {
                fileWriter.Write("  CPU/Memory Samples: {0:D}\n", m_CpuUsageStats.Count);
                fileWriter.Write("  CPU Usage max (%): {0:F2}\n", m_CpuUsageStats.MaxValue);
                fileWriter.Write("  CPU Usage min (%): {0:F2}\n", m_CpuUsageStats.MinValue);
                fileWriter.Write("  CPU Usage avg (%): {0:F2}\n", m_CpuUsageStats.Average);
                fileWriter.Write("  Memory Usage max (MB): {0:F2}\n", m_MemUsageStats.MaxValue);
                fileWriter.Write("  Memory Usage min (MB): {0:F2}\n", m_MemUsageStats.MinValue);
                fileWriter.Write("  Memory Usage avg (MB): {0:F2}\n", m_MemUsageStats.Average);
            }
            else
                Console.Write("No CPU/Mem statistics taken.\n\n");

            fileWriter.Write("  Process ID: {0}\n", Process.GetCurrentProcess().Id.ToString());
        }

        private void SendToLeastLoadedThread(IChannel chnl)
        {
            SessionHandler? connHandler = null;
            int minProviderConnCount, connHandlerIndex = 0;

            minProviderConnCount = 0x7fffffff;
            for (int i = 0; i < m_SessionHandlerCount; ++i)
            {
                SessionHandler tmpHandler = m_SessionHandlerList![i];
                tmpHandler.HandlerLock.EnterWriteLock();
                if (tmpHandler.OpenChannelsCount < minProviderConnCount)
                {
                    minProviderConnCount = tmpHandler.OpenChannelsCount;
                    connHandler = tmpHandler;
                    connHandlerIndex = i;
                }
                tmpHandler.HandlerLock.ExitWriteLock();
            }

            connHandler!.HandlerLock.EnterWriteLock();
            connHandler.OpenChannelsCount += 1;
            connHandler.NewChannelsList.Add(chnl);
            connHandler.HandlerLock.ExitWriteLock();

            Console.Write("Server: " + m_Server!.Socket.Handle);
            Console.Write("New Channel {0} passed connection to handler {1}\n\n", chnl.Socket.Handle, connHandlerIndex);
        }

        /* Prints the current time, in Coordinated Universal Time. */
        private void PrintCurrentTimeUTC(StreamWriter fileWriter)
        {
            DateTime dateTime = DateTime.Now;

            fileWriter.Write("{0:D}-{1,2:D}-{2,2:D} {3,2:D}:{4,2:D}:{5,2:D}",
                    dateTime.Year, dateTime.Month, dateTime.Day,
                    dateTime.Hour, dateTime.Minute, dateTime.Second);
        }

        private IServer? BindServer(out Error error)
        {
            IServer srvr;
            BindOptions sopts = new BindOptions();

            sopts.GuaranteedOutputBuffers = TransportPerfConfig.GuaranteedOutputBuffers;

            if (TransportPerfConfig.InterfaceName != null)
                sopts.InterfaceName = TransportPerfConfig.InterfaceName;

            sopts.MajorVersion = 0;
            sopts.MinorVersion = 0;
            sopts.ProtocolType = (ProtocolType)TransportThread.TEST_PROTOCOL_TYPE;
            sopts.TcpOpts.TcpNoDelay = TransportPerfConfig.TcpNoDelay;
            sopts.ConnectionType = TransportPerfConfig.ConnectionType;
            sopts.MaxFragmentSize = TransportPerfConfig.MaxFragmentSize;
            sopts.CompressionType = TransportPerfConfig.CompressionType;
            sopts.CompressionLevel = TransportPerfConfig.CompressionLevel;
            sopts.ServiceName = TransportPerfConfig.PortNo;
            sopts.SysRecvBufSize = TransportPerfConfig.RecvBufSize;
            sopts.SysSendBufSize = TransportPerfConfig.SendBufSize;

            if ((srvr = Transport.Bind(sopts, out error)) is null)
                return null;

            Console.WriteLine($"\nServer bound on port {srvr.PortNumber}");

            return srvr;
        }

        protected static void ShutdownHandler(object? sender, ConsoleCancelEventArgs args)
        {
            ShutDownApp = true;
        }

        static void Main(string[] args)
        {
            TransportPerf transportperf = new TransportPerf();
            Console.CancelKeyPress += new ConsoleCancelEventHandler(ShutdownHandler);
            transportperf.Initialize(args);
            transportperf.Run(args);
        }
    }
}




