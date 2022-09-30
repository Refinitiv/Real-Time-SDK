/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;
using System.Threading;

namespace Refinitiv.Eta.PerfTools.Common
{
    /// <summary>
    /// Maintains the provider application instance. 
    /// Has logic for keeping track of things like CPU/Mem usage, and the list of provider threads.
    /// </summary>
    public class Provider
    {
        private StreamWriter? m_SummaryFileWriter;                // Logs summary information, such as application inputs and final statistics.

        private string? m_SummaryFile = null;                     // summary file

        private ProviderType m_ProviderType;                     // Type of provider.  
        private ProviderThreadStats m_TotalStats;                // only used for multiple provider threads

        private ValueStatistics m_CpuUsageStats;                 // Sampled CPU statistics. 
        private ValueStatistics m_MemUsageStats;                 // Sampled memory usage statistics.

        private CountStat m_RefreshCount;                        // Counts refreshes sent (for all provider threads).
        private CountStat m_UpdateCount;                         // Counts updates sent (for all provider threads).
        private CountStat m_RequestCount;                        // Counts requests received (for all provider threads).
        private CountStat m_CloseCount;                          // Counts closes received (for all provider threads).
        private CountStat m_PostCount;                           // Counts posts received (for all provider threads).
        private CountStat m_GenMsgSentCount;                     // Counts generic msgs sent (for all provider threads).
        private CountStat m_GenMsgRecvCount;                     // Counts generic msgs received (for all provider threads).
        private CountStat m_latencyGenMsgSentCount;              // Counts latency generic msgs sent (for all provider threads). 
        private CountStat m_outOfBuffersCount;                   // Counts updates not sent due to lack of output buffers.
                                                                
        private CountStat m_msgSentCount;                        // Counts total messages sent.
        private CountStat m_bufferSentCount;                     // Counts total buffers sent(used with msgSentCount for packing statistics).

        private ResourceUsageStats m_resourceUsageStats;

        /// <summary>
        /// Threads launched for the provider
        /// </summary>
        public Thread[]? ThreadList { get; private set; }

        /// <summary>
        /// List of ProviderThreads
        /// </summary>
        public ProviderThread[]? ProviderThreadList { get; private set; }

        public Provider()
        {
            m_RefreshCount = new CountStat();
            m_UpdateCount = new CountStat();
            m_RequestCount = new CountStat();
            m_CloseCount = new CountStat();
            m_PostCount = new CountStat();
            m_GenMsgSentCount = new CountStat();
            m_GenMsgRecvCount = new CountStat();
            m_latencyGenMsgSentCount = new CountStat();
            m_outOfBuffersCount = new CountStat();
            m_msgSentCount = new CountStat();
            m_bufferSentCount = new CountStat();
            m_CpuUsageStats = new ValueStatistics();
            m_MemUsageStats = new ValueStatistics();
            m_TotalStats = new ProviderThreadStats();
            m_resourceUsageStats = new ResourceUsageStats();
        }

        /// <summary>
        /// Initializes provider statistics and creates a specific number provider threads 
        /// as configured by the application.
        /// </summary>
        /// <param name="createProviderThread">allows the specific subtype of ProviderThread to be created</param>
        /// <param name="providerType">the provider type</param>
        /// <param name="xmlMsgData">the xml msg data</param>
        /// <param name="summaryFileName">the summary file name</param>
        public void Init(Func<XmlMsgData, ProviderThread> createProviderThread, ProviderType providerType, XmlMsgData xmlMsgData, string summaryFileName)
        {
            m_ProviderType = providerType;
            m_RefreshCount.Init();
            m_UpdateCount.Init();
            m_RequestCount.Init();
            m_CloseCount.Init();
            m_PostCount.Init();
            m_GenMsgSentCount.Init();
            m_GenMsgRecvCount.Init();
            m_latencyGenMsgSentCount.Init();
            m_outOfBuffersCount.Init();
            m_msgSentCount.Init();
            m_bufferSentCount.Init();
            m_CpuUsageStats.Clear();
            m_MemUsageStats.Clear();

            ThreadList = new Thread[ProviderPerfConfig.ThreadCount];
            ProviderThreadList = new ProviderThread[ProviderPerfConfig.ThreadCount];

            for (int i = 0; i < ProviderPerfConfig.ThreadCount; i++)
            {
                ProviderThread provThread = createProviderThread(xmlMsgData);
                provThread.Init(i, providerType);
                ProviderThreadList[i] = provThread;
                ThreadList[i] = new Thread(new ThreadStart(provThread.Run));        
            }

            m_SummaryFile = summaryFileName;
            try
            {
                m_SummaryFileWriter = new StreamWriter(m_SummaryFile);
            }
            catch (Exception e)
            {
                Console.WriteLine($"Error: Failed to open summary file '{m_SummaryFile}'.\n Exception: {e.Message}");
                Environment.Exit(-1);
            }

            if (providerType == ProviderType.PROVIDER_INTERACTIVE)
            {
                m_SummaryFileWriter.WriteLine(ProviderPerfConfig.ConfigString);
            }
            m_SummaryFileWriter.Flush();
        }

        /// <summary>
        /// Starts available provider threads
        /// </summary>
        public void StartThreads()
        {
            if (ThreadList is not null)
            {
                foreach (Thread providerThread in ThreadList)
                {
                    providerThread.Start();
                }
            }
        }

        /// <summary>
        /// Performs cleanup of provider threads
        /// </summary>
        public void Cleanup()
        {
            if (ProviderThreadList is not null)
            {
                foreach (ProviderThread providerThread in ProviderThreadList)
                {
                    providerThread.Cleanup();
                }
            }

            WaitForThreads();
        }

        private void CollectGenMsgStats(ProviderThreadInfo provThreadInfo)
        {
            TimeRecordQueue latencyRecords = provThreadInfo.GenMsgLatencyRecords;
            while (!latencyRecords.Records.IsEmpty)
            {
                TimeRecord? record;
                if (latencyRecords.Records.TryDequeue(out record))
                {
                    double latency = (double)(record.EndTime - record.StartTime) / (double)record.Ticks;

                    provThreadInfo.Stats.IntervalGenMsgLatencyStats.Update(latency);
                    provThreadInfo.Stats.GenMsgLatencyStats.Update(latency);

                    if (ProviderPerfConfig.ThreadCount > 1)
                        m_TotalStats.GenMsgLatencyStats.Update(latency);

                    if (provThreadInfo.LatencyLogFile != null)
                        provThreadInfo.LatencyLogFileWriter!.Write("Gen, {0}, {1}, {2}\n", record.StartTime, record.EndTime, record.EndTime - record.StartTime);

                    latencyRecords.Pools.Enqueue(record);
                }
            }
        }

        /// <summary>
        /// Collects and writes provider statistics. 
        /// Stats will reflect changes from the previous call to this method.
        /// </summary>
        /// <param name="writeStats">if true, writes statistics to provider stats file</param>
        /// <param name="displayStats">if true, writes stats to stdout</param>
        /// <param name="currentRuntimeSec">current time</param>
        /// <param name="timePassedSec">time passed since last stats collection, used to calculate message rates</param>
        public void CollectStats(bool writeStats, bool displayStats, long currentRuntimeSec, long timePassedSec)
        {
            long refreshCount, 
                updateCount, 
                requestCount, 
                closeCount, 
                postCount, 
                genMsgSentCount, 
                genMsgRecvCount, 
                latencyGenMsgSentCount, 
                latencyGenMsgRecvCount, 
                outOfBuffersCount, 
                msgSentCount, 
                bufferSentCount;
            
            m_resourceUsageStats.Refresh();
            double processCpuLoad = m_resourceUsageStats.CurrentProcessCpuLoad();
            double memoryUsage = m_resourceUsageStats.CurrentMemoryUsage();
            if (timePassedSec != 0)
            {
                m_CpuUsageStats.Update(processCpuLoad);
                m_MemUsageStats.Update(memoryUsage);
            }

            for (int i = 0; i < ProviderPerfConfig.ThreadCount; ++i)
            {
                ProviderThread providerThread = ProviderThreadList![i];
                CollectGenMsgStats(providerThread.ProvThreadInfo!);

                if (providerThread.ProvThreadInfo!.LatencyLogFileWriter != null)
                {
                    providerThread.ProvThreadInfo.LatencyLogFileWriter.Flush();
                }
                ProviderThreadStats stats = providerThread.ProvThreadInfo.Stats;

                requestCount = providerThread.ItemRequestCount.GetChange();
                refreshCount = providerThread.RefreshMsgCount.GetChange();
                updateCount = providerThread.UpdateMsgCount.GetChange();
                closeCount = providerThread.CloseMsgCount.GetChange();
                postCount = providerThread.PostMsgCount.GetChange();
                genMsgSentCount = stats.GenMsgSentCount.GetChange();
                genMsgRecvCount = stats.GenMsgRecvCount.GetChange();
                latencyGenMsgSentCount = stats.LatencyGenMsgSentCount.GetChange();
                latencyGenMsgRecvCount = stats.IntervalGenMsgLatencyStats.Count;
                outOfBuffersCount = providerThread.OutOfBuffersCount.GetChange();

                if (writeStats)
                {
                    // Write stats to the stats file.
                    var nowTime = System.DateTime.UtcNow;
                    providerThread.ProvThreadInfo.StatsFileWriter!.Write(nowTime.ToString());

                    switch (m_ProviderType)
                    {
                        case ProviderType.PROVIDER_INTERACTIVE:
                            providerThread.ProvThreadInfo.StatsFileWriter.Write(
                                    ", {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8:0.0}, {9:0.0}, {10:0.0}, {11:0.0}, {12:0.00}, {13:0.00}\n",
                                    requestCount,
                                    refreshCount,
                                    updateCount,
                                    postCount,
                                    genMsgSentCount,
                                    genMsgRecvCount,
                                    latencyGenMsgSentCount,
                                    latencyGenMsgRecvCount,
                                    stats.IntervalGenMsgLatencyStats.Average,
                                    Math.Sqrt(stats.IntervalGenMsgLatencyStats.Variance),
                                    (stats.IntervalGenMsgLatencyStats.Count > 0) ? stats.IntervalGenMsgLatencyStats.MaxValue : 0.0,
                                    (stats.IntervalGenMsgLatencyStats.Count > 0) ? stats.IntervalGenMsgLatencyStats.MinValue : 0.0,
                                    processCpuLoad,
                                    memoryUsage);
                            break;
                        case ProviderType.PROVIDER_NONINTERACTIVE:
                            providerThread.ProvThreadInfo.StatsFileWriter.Write(
                                    ", {0}, {1}, {2}, {3:0.00}, {4:0.00}\n",
                                    requestCount,
                                    refreshCount,
                                    updateCount,
                                    processCpuLoad,
                                    memoryUsage);
                            break;
                        default:
                            break;
                    }
                    providerThread.ProvThreadInfo.StatsFileWriter.Flush();
                }

                //Add the new counts to the provider's total.
                m_RequestCount.Add(requestCount);
                m_UpdateCount.Add(updateCount);
                m_RefreshCount.Add(refreshCount);
                m_CloseCount.Add(closeCount);
                m_PostCount.Add(postCount);
                m_GenMsgSentCount.Add(genMsgSentCount);
                m_GenMsgRecvCount.Add(genMsgRecvCount);
                m_latencyGenMsgSentCount.Add(latencyGenMsgSentCount);
                m_outOfBuffersCount.Add(outOfBuffersCount);

                //Take packing stats, if packing is enabled.
                if (ProviderPerfConfig.TotalBuffersPerPack > 1)
                {
                    m_msgSentCount.Add(providerThread.MsgSentCount.GetChange());
                    m_bufferSentCount.Add(providerThread.BufferSentCount.GetChange());
                }

                if (displayStats)
                {
                    //Print screen stats.
                    if (ProviderPerfConfig.ThreadCount == 1)
                        Console.Write("{0:000}: ", currentRuntimeSec);
                    else
                        Console.Write("{0:000}: Thread {1}\n  ", currentRuntimeSec, i + 1);

                    Console.Write("UpdRate: {0}, CPU: {1:0.00}%, Mem: {2:0.00}MB\n",
                                      updateCount / timePassedSec,
                                      processCpuLoad, memoryUsage);

                    switch (m_ProviderType)
                    {
                        case ProviderType.PROVIDER_INTERACTIVE:
                            if (requestCount > 0 || refreshCount > 0)
                                Console.Write("  - Received {0} item requests (total: {1}), sent {2} images (total: {3})\n",
                                                  requestCount,
                                                  m_RequestCount.GetTotal(),
                                                  refreshCount,
                                                  m_RefreshCount.GetTotal());
                            if (postCount > 0)
                                Console.Write("  Posting: received {0}, reflected {1}\n", postCount, postCount);
                            if (genMsgRecvCount > 0 || genMsgSentCount > 0)
                            {
                                Console.Write("  GenMsgs: sent {0}, received {1}, latencies sent {2}, latencies received {3}\n",
                                        genMsgSentCount, genMsgRecvCount, latencyGenMsgSentCount, latencyGenMsgRecvCount);
                            }
                            if (stats.IntervalGenMsgLatencyStats.Count > 0)
                            {
                                stats.IntervalGenMsgLatencyStats.Print("  GenMsgLat(usec)", "Msgs", false);
                                stats.IntervalGenMsgLatencyStats.Clear();
                            }

                            break;
                        case ProviderType.PROVIDER_NONINTERACTIVE:
                            if (requestCount > 0 || refreshCount > 0)
                                Console.Write("  - Sent {0} images (total: {1})\n", refreshCount, m_RefreshCount.GetTotal());
                            break;
                        default:
                            break;
                    }

                    closeCount = m_CloseCount.GetChange();
                    if (closeCount > 0)
                        Console.Write("  - Received {0} closes.\n", closeCount);

                    outOfBuffersCount = m_outOfBuffersCount.GetChange();

                    if (outOfBuffersCount > 0)
                        Console.Write("  - Stopped {0} updates due to lack of output buffers.\n", outOfBuffersCount);

                    // Print packing stats, if packing is enabled. 
                    msgSentCount = m_msgSentCount.GetChange();
                    bufferSentCount = m_bufferSentCount.GetChange();
                    if (bufferSentCount > 0)
                    {
                        Console.Write("  - Approx. avg msgs per pack: {0:0}\n", (double)msgSentCount / (double)bufferSentCount);
                    }
                }
            }
        }

        /// <summary>
        /// Prints summary statistics to the summary file
        /// </summary>
        /// <param name="fileWriter">file to write summary stats to</param>
        public void PrintSummaryStats(StreamWriter fileWriter)
        {
            long statsTime = 0;
            long currentTime = (long)GetTime.GetNanoseconds();

            if (ProviderPerfConfig.ThreadCount > 1)
            {
                m_TotalStats.InactiveTime = ProviderThreadList![0].ProvThreadInfo!.Stats.InactiveTime;
                m_TotalStats.FirstGenMsgSentTime = ProviderThreadList![0].ProvThreadInfo!.Stats.FirstGenMsgSentTime;
                m_TotalStats.FirstGenMsgRecvTime = ProviderThreadList![0].ProvThreadInfo!.Stats.FirstGenMsgRecvTime;
                for (int i = 0; i < ProviderPerfConfig.ThreadCount; ++i)
                {
                    ProviderThread providerThread = ProviderThreadList[i];
                    ProviderThreadStats stats = providerThread.ProvThreadInfo!.Stats;
                    statsTime = (stats.InactiveTime > 0 && stats.InactiveTime < currentTime) ? stats.InactiveTime : currentTime;
                    if (stats.InactiveTime > 0 && stats.InactiveTime < m_TotalStats.InactiveTime)
                        m_TotalStats.InactiveTime = stats.InactiveTime;
                    if (stats.FirstGenMsgSentTime != 0 && stats.FirstGenMsgSentTime < m_TotalStats.FirstGenMsgSentTime)
                        m_TotalStats.FirstGenMsgSentTime = stats.FirstGenMsgSentTime;
                    if (stats.FirstGenMsgRecvTime != 0 && stats.FirstGenMsgRecvTime < m_TotalStats.FirstGenMsgRecvTime)
                        m_TotalStats.FirstGenMsgSentTime = stats.FirstGenMsgRecvTime;
                    fileWriter.Write("\n--- THREAD {0} SUMMARY ---\n\n", i + 1);

                    fileWriter.Write("Overall Statistics: \n");

                    switch (m_ProviderType)
                    {
                        case ProviderType.PROVIDER_INTERACTIVE:
                            if (m_TotalStats.GenMsgLatencyStats.Count > 0)
                            {
                                fileWriter.Write($"  GenMsg latency avg (usec): {stats.GenMsgLatencyStats.Average:0.0}\n" +
                                                 $"  GenMsg latency std dev (usec): {Math.Sqrt(stats.GenMsgLatencyStats.Variance):0.0}\n" +
                                                 $"  GenMsg latency max (usec): {(stats.GenMsgLatencyStats.Count > 0 ? stats.GenMsgLatencyStats.MaxValue : 0):0.0}\n" +
                                                 $"  GenMsg latency min (usec): {(stats.GenMsgLatencyStats.Count > 0 ? stats.GenMsgLatencyStats.MinValue : 0):0.0}\n");
                            }
                            else
                                fileWriter.Write($"  No GenMsg latency information was received.\n");
                            if (ProviderPerfConfig.GenMsgsPerSec > 0)
                                fileWriter.Write($"  GenMsgs sent: {0}\n", stats.GenMsgSentCount.GetTotal());
                            if (stats.GenMsgRecvCount.GetTotal() > 0)
                                fileWriter.Write($"  GenMsgs received: {0}\n", stats.GenMsgRecvCount.GetTotal());
                            if (ProviderPerfConfig.LatencyGenMsgRate > 0)
                                fileWriter.Write($"  GenMsg latencies sent: {stats.LatencyGenMsgSentCount.GetTotal()}\n");
                            if (stats.GenMsgLatencyStats.Count > 0)
                                fileWriter.Write($"  GenMsg latencies received: {stats.GenMsgLatencyStats.Count}\n");
                            if (ProviderPerfConfig.GenMsgsPerSec > 0)
                            {
                                fileWriter.Write($"  Avg GenMsg send rate: {stats.GenMsgSentCount.GetTotal() / ((statsTime - stats.FirstGenMsgSentTime) / 1000000000.0):0}\n");
                            }
                            if (stats.GenMsgRecvCount.GetTotal() > 0)
                            {
                                fileWriter.Write($"  Avg GenMsg receive rate: {stats.GenMsgRecvCount.GetTotal() / ((statsTime - stats.FirstGenMsgRecvTime) / 1000000000.0):0}\n");
                            }
                            if (ProviderPerfConfig.LatencyGenMsgRate > 0)
                            {
                                fileWriter.Write($"  Avg GenMsg latency send rate: {stats.LatencyGenMsgSentCount.GetTotal() / ((statsTime - stats.FirstGenMsgSentTime) / 1000000000.0):0}\n");
                            }
                            if (stats.GenMsgLatencyStats.Count > 0)
                            {
                                fileWriter.Write($"  Avg GenMsg latency receive rate: {stats.GenMsgLatencyStats.Count / ((statsTime - stats.FirstGenMsgRecvTime) / 1000000000.0):0}\n");
                            }
                            fileWriter.Write($"  Image requests received: {providerThread.ItemRequestCount.GetTotal()}\n");
                            if (ProviderPerfConfig.UpdatesPerSec > 0)
                                fileWriter.Write($"  Updates sent: {providerThread.UpdateMsgCount.GetTotal()}\n");
                            if (providerThread.PostMsgCount.GetTotal() > 0)
                            {
                                fileWriter.Write($"  Posts received: {providerThread.PostMsgCount.GetTotal()}\n");
                                fileWriter.Write($"  Posts reflected: {providerThread.PostMsgCount.GetTotal()}\n");
                            }
                            break;
                        case ProviderType.PROVIDER_NONINTERACTIVE:
                            fileWriter.Write(
                                    $"  Images sent: {providerThread.RefreshMsgCount.GetTotal()}\n" +
                                    $"  Updates sent: {providerThread.UpdateMsgCount.GetTotal()}\n");
                            break;
                        default:
                            break;
                    }
                }
            }
            else
            {
                m_TotalStats = ProviderThreadList![0].ProvThreadInfo!.Stats;
                statsTime = (m_TotalStats.InactiveTime != 0 && m_TotalStats.InactiveTime < currentTime) ? m_TotalStats.InactiveTime : currentTime;
            }
            fileWriter.Write("\n--- OVERALL SUMMARY ---\n\n");

            fileWriter.Write("Overall Statistics: \n");

            switch (m_ProviderType)
            {
                case ProviderType.PROVIDER_INTERACTIVE:
                    if (m_TotalStats.GenMsgLatencyStats.Count > 0)
                    {
                        fileWriter.Write($"  GenMsg latency avg (usec): {m_TotalStats.GenMsgLatencyStats.Average:0.0}\n" +
                                         $"  GenMsg latency std dev (usec): {Math.Sqrt(m_TotalStats.GenMsgLatencyStats.Variance):0.0}\n" +
                                         $"  GenMsg latency max (usec): {m_TotalStats.GenMsgLatencyStats.MaxValue:0.0}\n" +
                                         $"  GenMsg latency min (usec): {m_TotalStats.GenMsgLatencyStats.MinValue:0.0}\n");
                    }
                    else
                        fileWriter.Write("  No GenMsg latency information was received.\n");
                    if (ProviderPerfConfig.GenMsgsPerSec > 0)
                        fileWriter.Write($"  GenMsgs sent: {m_GenMsgSentCount.GetTotal()}\n");
                    if (m_GenMsgRecvCount.GetTotal() > 0)
                        fileWriter.Write($"  GenMsgs received: {m_GenMsgRecvCount.GetTotal()}\n");
                    if (ProviderPerfConfig.LatencyGenMsgRate > 0)
                        fileWriter.Write($"  GenMsg latencies sent: {m_latencyGenMsgSentCount.GetTotal()}\n");
                    if (m_TotalStats.GenMsgLatencyStats.Count > 0)
                        fileWriter.Write($"  GenMsg latencies received: {m_TotalStats.GenMsgLatencyStats.Count}\n");
                    if (ProviderPerfConfig.GenMsgsPerSec > 0)
                    {
                        fileWriter.Write($"  Avg GenMsg send rate: {m_GenMsgSentCount.GetTotal() / ((statsTime - m_TotalStats.FirstGenMsgSentTime) / 1000000000.0):0}\n");
                    }
                    if (m_GenMsgRecvCount.GetTotal() > 0)
                    {
                        fileWriter.Write($"  Avg GenMsg receive rate: {m_GenMsgRecvCount.GetTotal() / ((statsTime - m_TotalStats.FirstGenMsgRecvTime) / 1000000000.0):0}\n");
                    }
                    if (ProviderPerfConfig.LatencyGenMsgRate > 0)
                    {
                        fileWriter.Write($"  Avg GenMsg latency send rate: {m_latencyGenMsgSentCount.GetTotal() / ((statsTime - m_TotalStats.FirstGenMsgSentTime) / 1000000000.0):0}\n");
                    }
                    if (m_TotalStats.GenMsgLatencyStats.Count > 0)
                    {
                        fileWriter.Write($"  Avg GenMsg latency receive rate: {m_TotalStats.GenMsgLatencyStats.Count / ((statsTime - m_TotalStats.FirstGenMsgRecvTime) / 1000000000.0):0}\n");
                    }
                    fileWriter.Write("  Image requests received: {0}\n", m_RequestCount.GetTotal());
                    if (ProviderPerfConfig.UpdatesPerSec > 0)
                        fileWriter.Write("  Updates sent: {0}\n", m_UpdateCount.GetTotal());
                    if (m_PostCount.GetTotal() > 0)
                    {
                        fileWriter.Write($"  Posts received: {m_PostCount.GetTotal()}\n");
                        fileWriter.Write($"  Posts reflected: {m_PostCount.GetTotal()}\n");
                    }
                    break;
                case ProviderType.PROVIDER_NONINTERACTIVE:
                    fileWriter.Write($"  Image sent: {m_RefreshCount.GetTotal()}\n Updates sent: {m_UpdateCount.GetTotal()}\n");
                    break;
                default:
                    break;
            }

            if (m_CpuUsageStats.Count > 0)
            {
                fileWriter.Write($"  CPU/Memory Samples: {m_CpuUsageStats.Count}\n");
                fileWriter.Write($"  CPU Usage max (%): {m_CpuUsageStats.MaxValue:0.00}\n");
                fileWriter.Write($"  CPU Usage min (%): {m_CpuUsageStats.MinValue:0.00}\n");
                fileWriter.Write($"  CPU Usage avg (%): {m_CpuUsageStats.Average:0.00}\n");
                fileWriter.Write($"  Memory Usage max (MB): {m_MemUsageStats.MaxValue:0.00}\n");
                fileWriter.Write($"  Memory Usage min (MB): {m_MemUsageStats.MinValue:0.00}\n");
                fileWriter.Write($"  Memory Usage avg (MB): {m_MemUsageStats.Average:0.00}\n");
            }

            fileWriter.Write("\n");
        }

        /// <summary>
        /// Prints the final statsistics
        /// </summary>
        public void PrintFinalStats()
        {
            StreamWriter printWriter = new StreamWriter(Console.OpenStandardOutput());
            PrintSummaryStats(printWriter);
            printWriter.Flush();
            PrintSummaryStats(m_SummaryFileWriter!);
            m_SummaryFileWriter!.Close();
            printWriter.Close();
        }

        /// <summary>
        /// Wait for provider threads to die
        /// </summary>
        private void WaitForThreads()
        {
            if (ThreadList is not null)
            {
                for (int i = 0; i < ThreadList.Length; ++i)
                {
                    ThreadList[i].Join(1000);
                }
            }

            ThreadList = null;
            ProviderThreadList = null;
        }
    }
}
