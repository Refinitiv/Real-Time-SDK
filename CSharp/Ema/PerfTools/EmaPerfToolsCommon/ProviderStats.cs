/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using static LSEG.Ema.Access.OmmProviderConfig;

namespace LSEG.Ema.PerfTools.Common
{
    public class ProviderStats
    {
        private ProviderThreadStats totalStats;

        private ResourceUsageStats m_ResourceUsageStats = new ResourceUsageStats();

        private ValueStatistics cpuUsageStatistics;
        private ValueStatistics memUsageStatistics;

        private BaseProviderPerfConfig config;
        private ProviderThread[] providerThreads;
        private Thread[] threads;
        private LogFileInfo? summaryFile;

        private CountdownEvent m_RunLoopExited = new CountdownEvent(1);

        private volatile bool m_ExitApp;

#pragma warning disable CS8618
        public ProviderStats()
#pragma warning restore CS8618
        {
            totalStats = new ProviderThreadStats();
            cpuUsageStatistics = new ValueStatistics();
            memUsageStatistics = new ValueStatistics();
        }

        public void Initialize(BaseProviderPerfConfig config, Func<ProviderThread> createProviderThread)
        {
            this.config = config;
            cpuUsageStatistics.Clear();
            memUsageStatistics.Clear();

            providerThreads = new ProviderThread[config.ThreadCount];

            for (int i = 0; i < config.ThreadCount; i++)
            {
                providerThreads[i] = createProviderThread();
                providerThreads[i].Initialize(i);
            }

            Console.WriteLine(config.ToString());

            if (!string.IsNullOrEmpty(config.SummaryFilename))
            {
                summaryFile = LogFileHelper.InitFile(config.SummaryFilename);
            }
            summaryFile.WriteFile(config.ToString());
        }

        public void Run()
        {
            RegisterShutdownHook();
            StartThreads();
            long nextTime = (long)GetTime.GetMilliseconds() + 1000L;
            int intervalSeconds = 0;
            int currentRuntimeSec = 0;
            int writeStatsInterval = config.WriteStatsInterval;
            int runTime = config.RunTime;
            bool displayStats = config.DisplayStats;

            do
            {
                if (GetTime.GetMilliseconds() >= nextTime)
                {
                    nextTime += 1000;
                    ++intervalSeconds;
                    ++currentRuntimeSec;

                    // Check if it's time to print stats
                    if (intervalSeconds >= writeStatsInterval)
                    {
                        CollectStats(true, displayStats, currentRuntimeSec, writeStatsInterval);
                        intervalSeconds = 0;
                    }
                }

                try
                {
                    long sleepTime = nextTime - (long)GetTime.GetMilliseconds();
                    if (sleepTime > 0)
                    {
                        Thread.Sleep((int)sleepTime);
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }
            } while (currentRuntimeSec < runTime && !m_ExitApp);

            Console.WriteLine();
            Console.WriteLine(
                m_ExitApp
                    ? "User requested shutdown."
                    : $"Run time of {runTime} seconds has expired.");

            Stop();
            m_RunLoopExited.Signal(1);
        }

        public void StartThreads()
        {
            threads = new Thread[providerThreads.Length];
            for (var i = 0; i < providerThreads.Length; i++)
            {
                var providerThread = providerThreads[i];
                var thread = new Thread(new ThreadStart(providerThread.Run)) { Name = $"{providerThread.providerConfigName} Worker" };
                thread.Start();
                threads[i] = thread;
            }
        }

        /// <summary>
        /// Prints the current UTC time
        /// </summary>
        /// <param name="fileWriter">the stream writer for statistics output</param>
        private void PrintCurrentTimeUTC(LogFileInfo? logFileInfo)
        {
            var nowTime = System.DateTime.UtcNow;
            logFileInfo.WriteFile(nowTime.ToString());
        }

        public void StopThreads()
        {
            Console.WriteLine("Shutting down...");

            if (threads == null || threads.Length == 0)
                return;

            foreach (ProviderThread providerThread in providerThreads)
            {
                providerThread.shutdown = true;
            }

            foreach (var thread in threads)
            {
                var threadFinished = false;
                var shutdownCount = 0;
                while (!threadFinished && shutdownCount < 3)
                {
                    try
                    {
                        threadFinished = thread.Join(1000);
                        shutdownCount++;
                    }
                    catch (Exception e)
                    {
                        Console.Error.WriteLine($"Thread.Join failed with exception {e}");
                        Environment.Exit(-1);
                    }
                }
            }
        }

        public void Clear()
        {
            foreach (ProviderThread thread in providerThreads)
            {
                thread.Clear();
            }
            summaryFile.Close(); // TODO: Make LogFileInfo class IDisposable instead
        }

        /// <summary>
        /// Collects and writes provider statistics. 
        /// Stats will reflect changes from the previous call to this method.
        /// </summary>
        /// <param name="writeStats">if true, writes statistics to provider stats file</param>
        /// <param name="displayStats">if true, writes stats to stdout.</param>
        /// <param name="currentRuntimeSec">current time</param>
        /// <param name="timePassedSec">time passed since last stats collection, used to calculate message rates.</param>
        public void CollectStats(bool writeStats, bool displayStats, long currentRuntimeSec, long timePassedSec)
        {
            long refreshCount, updateCount, requestCount, closeCount, postCount, statusCount, genMsgSentCount, genMsgRecvCount, 
                latencyGenMsgSentCount, latencyGenMsgRecvCount, outOfBuffersCount, itemRefreshCount, updatePackedMsgCount;

            m_ResourceUsageStats.Refresh();
            double processCpuLoad = m_ResourceUsageStats.CurrentProcessCpuLoad();
            double memoryUsage = m_ResourceUsageStats.CurrentMemoryUsage();
            if (timePassedSec != 0)
            {
                cpuUsageStatistics.Update(processCpuLoad);
                memUsageStatistics.Update(memoryUsage);
            }

            for (int i = 0; i < config.ThreadCount; ++i)
            {
                ProviderThread providerThread = providerThreads[i];
                CollectGenMsgStats(providerThread.ProviderThreadStats, providerThread);

                ProviderThreadStats stats = providerThread.ProviderThreadStats;

                requestCount = stats.RequestCount.GetChange();
                refreshCount = stats.RefreshCount.GetChange();
                itemRefreshCount = stats.ItemRefreshCount.GetChange();
                updateCount = stats.UpdateCount.GetChange();
                closeCount = stats.CloseCount.GetChange();
                postCount = stats.PostCount.GetChange();
                statusCount = stats.StatusCount.GetChange();
                genMsgSentCount = stats.GenMsgSentCount.GetChange();
                genMsgRecvCount = stats.GenMsgRecvCount.GetChange();
                latencyGenMsgSentCount = stats.LatencyGenMsgSentCount.GetChange();
                latencyGenMsgRecvCount = stats.IntervalGenMsgLatencyStats.Count;
                outOfBuffersCount = stats.OutOfBuffersCount.GetChange();
                updatePackedMsgCount = stats.UpdatePackedMsgCount.GetChange();

                if (writeStats)
                {
                    PrintCurrentTimeUTC(providerThread.statsFile);
                    if (providerThread.ProviderRoleKind == ProviderRoleEnum.INTERACTIVE)
                    {
                        var latencyGenMsgRecvStats = stats.IntervalGenMsgLatencyStats;
                        var printMinAndMax = latencyGenMsgRecvCount > 0;
                        providerThread.statsFile.WriteFile(string.Format(
                            ",  {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8:0.0}, {9:0.0}, {10:0.0}, {11:0.0}, {12:0.00}, {13:0.00}\n",
                            requestCount,
                            refreshCount,
                            updateCount,
                            postCount,
                            genMsgSentCount,
                            genMsgRecvCount,
                            latencyGenMsgSentCount,
                            latencyGenMsgRecvCount,
                            latencyGenMsgRecvStats.Average,
                            latencyGenMsgRecvStats.StandardDeviation,
                            printMinAndMax ? latencyGenMsgRecvStats.MaxValue : 0.0,
                            printMinAndMax ? latencyGenMsgRecvStats.MinValue : 0.0,
                            processCpuLoad,
                            memoryUsage));
                    }
                    else
                    {
                        providerThread.statsFile.WriteFile(string.Format(
                            ", {0}, {1}, {2}, {3:0.00}, {4:0.00}\n",
                            requestCount,
                            refreshCount,
                            updateCount,
                            processCpuLoad,
                            memoryUsage));
                    }
                }

                //Add the new counts to the provider's total.
                totalStats.RequestCount.Add(requestCount);
                totalStats.UpdateCount.Add(updateCount);
                totalStats.RefreshCount.Add(refreshCount);
                totalStats.ItemRefreshCount.Add(itemRefreshCount);
                totalStats.CloseCount.Add(closeCount);
                totalStats.PostCount.Add(postCount);
                totalStats.StatusCount.Add(statusCount);
                totalStats.GenMsgSentCount.Add(genMsgSentCount);
                totalStats.GenMsgRecvCount.Add(genMsgRecvCount);
                totalStats.LatencyGenMsgSentCount.Add(latencyGenMsgSentCount);
                totalStats.OutOfBuffersCount.Add(outOfBuffersCount);
                totalStats.UpdatePackedMsgCount.Add(updatePackedMsgCount);

                if (displayStats)
                {
                    //Print screen stats.
                    if (config.ThreadCount == 1)
                    {
                        Console.Write($"{currentRuntimeSec}: ");
                    }
                    else
                    {
                        Console.WriteLine($"{currentRuntimeSec}: Thread {i + 1}:");
                        Console.Write("  ");
                    }

                    Console.WriteLine($"UpdRate: {updateCount / timePassedSec,8}, CPU: {processCpuLoad,6:0.00}%, Mem: {memoryUsage,6:0.00} MB");

                    if (requestCount > 0 || refreshCount > 0)
                    {
                        if (providerThread.ProviderRoleKind == ProviderRoleEnum.INTERACTIVE)
                        {
                            Console.WriteLine($"  - Sent {itemRefreshCount} images (total: {totalStats.ItemRefreshCount.GetTotal()})");
                        }
                        else
                        {
                            Console.WriteLine($"  - Received {requestCount} item requests (total: {totalStats.RefreshCount.GetTotal()}," +
                                $" sent {refreshCount} images (total: {totalStats.ItemRefreshCount.GetTotal()}");
                        }
                    }

                    if (updatePackedMsgCount > 0 && ((updateCount / updatePackedMsgCount) > 1))
                    {
                        Console.WriteLine($"  Average update messages packed per message: {(updateCount / updatePackedMsgCount)}");
                    }

                    if (postCount > 0)
                    {
                        Console.WriteLine($"  Posting: received {postCount}, reflected {postCount}");
                    }

                    if (genMsgRecvCount > 0 || genMsgSentCount > 0)
                    {
                        Console.WriteLine($"  GenMsgs: sent {genMsgSentCount}, received {genMsgRecvCount}, latencies sent {latencyGenMsgSentCount}, latencies received {latencyGenMsgRecvCount}");
                    }

                    if (stats.IntervalGenMsgLatencyStats.Count > 0)
                    {
                        stats.IntervalGenMsgLatencyStats.Print("  GenMsgLat(usec)", "Msgs", false);
                        stats.IntervalGenMsgLatencyStats.Clear();
                    }

                    closeCount = totalStats.CloseCount.GetChange();
                    if (closeCount > 0)
                    {
                        Console.WriteLine($"  - Received {closeCount} closes.");
                    }

                    outOfBuffersCount = totalStats.OutOfBuffersCount.GetChange();
                    if (outOfBuffersCount > 0)
                    {
                        Console.WriteLine($"  - Stopped {outOfBuffersCount} updates due to lack of output buffers.");
                    }
                }
            }
        }

        /// <summary>
        /// Prints summary stats to the summary file.
        /// </summary>
        /// <param name="fileWriter">file to write summary stats to.</param>
        public void PrintSummaryStats(TextWriter fileWriter)
        {
            long statsTime = 0;
            double currentTime = GetTime.GetMicroseconds();

            if (config.ThreadCount > 1)
            {
                totalStats.InactiveTime = providerThreads[0].ProviderThreadStats.InactiveTime;
                totalStats.FirstGenMsgSentTime = providerThreads[0].ProviderThreadStats.FirstGenMsgSentTime;
                totalStats.FirstGenMsgRecvTime = providerThreads[0].ProviderThreadStats.FirstGenMsgRecvTime;
                for (int i = 0; i < config.ThreadCount; ++i)
                {
                    ProviderThread providerThread = providerThreads[i];
                    ProviderThreadStats stats = providerThread.ProviderThreadStats;
                    statsTime = (stats.InactiveTime > 0 && stats.InactiveTime < currentTime) ? stats.InactiveTime : (long)currentTime;
                    if (stats.InactiveTime > 0 && stats.InactiveTime < totalStats.InactiveTime)
                    {
                        totalStats.InactiveTime = stats.InactiveTime;
                    }

                    fileWriter.WriteLine();
                    fileWriter.WriteLine($"--- THREAD {i + 1} SUMMARY ---");
                    fileWriter.WriteLine();

                    fileWriter.WriteLine("Overall Statistics:");

                    if (providerThread.ProviderRoleKind == ProviderRoleEnum.INTERACTIVE)
                    {
                        if (stats.FirstGenMsgSentTime != 0 && stats.FirstGenMsgSentTime < totalStats.FirstGenMsgSentTime)
                        {
                            totalStats.FirstGenMsgSentTime = stats.FirstGenMsgSentTime;
                        }

                        if (stats.FirstGenMsgRecvTime != 0 && stats.FirstGenMsgRecvTime < totalStats.FirstGenMsgRecvTime)
                        {
                            totalStats.FirstGenMsgRecvTime = stats.FirstGenMsgRecvTime;
                        }

                        if (totalStats.GenMsgLatencyStats.Count > 0)
                        {
                            PrintValueAvgStdDevMaxMin(stats.GenMsgLatencyStats, "GenMsg latency", "usec");
                        }
                        else
                        {
                            fileWriter.WriteLine("  No GenMsg latency information was received.");
                        }

                        if (stats.GenMsgSentCount.GetTotal() > 0)
                        {
                            fileWriter.WriteLine($"  GenMsgs sent: {stats.GenMsgSentCount.GetTotal()}");
                        }

                        if (stats.GenMsgRecvCount.GetTotal() > 0)
                        {
                            fileWriter.WriteLine($"  GenMsgs received: {stats.GenMsgRecvCount.GetTotal()}");
                        }

                        if (stats.LatencyGenMsgSentCount.GetTotal() > 0)
                        {
                            fileWriter.WriteLine($"  GenMsg latencies sent: {stats.LatencyGenMsgSentCount.GetTotal()}");
                        }

                        if (stats.GenMsgLatencyStats.Count > 0)
                        {
                            fileWriter.WriteLine($"  GenMsg latencies received: {stats.GenMsgLatencyStats.Count}");
                        }

                        PrintAvgRate("GenMsg send", stats.GenMsgSentCount.GetTotal(), stats.FirstGenMsgSentTime, statsTime);
                        PrintAvgRate("GenMsg receive", stats.GenMsgRecvCount.GetTotal(), stats.FirstGenMsgRecvTime, statsTime);
                        PrintAvgRate("GenMsg latency send", stats.LatencyGenMsgSentCount.GetTotal(), stats.FirstGenMsgSentTime, statsTime);
                        PrintAvgRate("GenMsg latency receive", stats.GenMsgLatencyStats.Count, stats.FirstGenMsgRecvTime, statsTime);

                        fileWriter.WriteLine($"  Image requests received: {stats.RequestCount.GetTotal()}");
                    }

                    if (providerThread.ProviderRoleKind == ProviderRoleEnum.NON_INTERACTIVE)
                    {
                        fileWriter.WriteLine($"  Images sent: {stats.ItemRefreshCount.GetTotal()}");
                    }

                    if (config.UpdatesPerSec > 0)
                    {
                        fileWriter.WriteLine($"  Updates sent: {stats.UpdateCount.GetTotal()}");
                    }

                    if (stats.PostCount.GetTotal() > 0)
                    {
                        PrintPostsCount(stats);
                    }

                    if (stats.UpdatePackedMsgCount.GetTotal() > 0)
                    {
                        fileWriter.WriteLine($"  Packed Update Messages Sent: {stats.UpdatePackedMsgCount.GetTotal()}");
                    }
                }
            }
            else
            {
                ProviderThreadStats stats = providerThreads[0].ProviderThreadStats;
                statsTime = (stats.InactiveTime != 0 && stats.InactiveTime < currentTime) ? stats.InactiveTime : (long)currentTime;
            }
            fileWriter.WriteLine();
            fileWriter.WriteLine("--- OVERALL SUMMARY ---");
            fileWriter.WriteLine();

            fileWriter.WriteLine("Overall Statistics:");

            if (totalStats.GenMsgLatencyStats.Count > 0)
            {
                PrintValueAvgStdDevMaxMin(totalStats.GenMsgLatencyStats, "GenMsg latency", "usec");
            }
            else
            {
                fileWriter.WriteLine("  No GenMsg latency information was received.");
            }


            if (totalStats.GenMsgLatencyStats.Count > 0)
            {
                fileWriter.WriteLine("  Avg GenMsg latency receive rate: {0:0.0}", totalStats.GenMsgLatencyStats.Count /
                        ((statsTime - totalStats.FirstGenMsgRecvTime) / 1000000.0));
            }

            if (providerThreads[0].ProviderRoleKind == Access.OmmProviderConfig.ProviderRoleEnum.NON_INTERACTIVE)
            {
                fileWriter.WriteLine($"  Image requests sent: {totalStats.ItemRefreshCount.GetTotal()}");
            }
            else
            {
                fileWriter.WriteLine($"  Image requests received: {totalStats.RequestCount.GetTotal()}");
            }

            if (config.UpdatesPerSec > 0)
            {
                fileWriter.WriteLine($"  Updates sent: {totalStats.UpdateCount.GetTotal()}");
            }

            if (totalStats.PostCount.GetTotal() > 0)
            {
                PrintPostsCount(totalStats);
            }

            if (totalStats.UpdatePackedMsgCount.GetTotal() > 0)
            {
                fileWriter.WriteLine($"  Packed Update Messages Sent: {totalStats.UpdatePackedMsgCount.GetTotal()}");
                fileWriter.WriteLine($"  Average Update Messages Packed per Packed Message Send: {totalStats.UpdateCount.GetTotal()/totalStats.UpdatePackedMsgCount.GetTotal()}");
            }

            if (cpuUsageStatistics.Count > 0)
            {
                fileWriter.WriteLine($"  CPU/Memory Samples: {cpuUsageStatistics.Count}");
                PrintValueMaxMinAvg(cpuUsageStatistics, "CPU Usage", "%");
                PrintValueMaxMinAvg(memUsageStatistics, "Memory Usage", "MB");
            }

            fileWriter.WriteLine();

            void PrintPostsCount(ProviderThreadStats providerThreadStats)
            {
                // Post message is being resent to consumer & if something goes wrong application would fail and wouldn't output any stats,
                // otherwise those "received" & "reflected" are equal.
                var value = providerThreadStats.PostCount.GetTotal();
                fileWriter.WriteLine($"  Posts received: {value}");
                fileWriter.WriteLine($"  Posts reflected: {value}");
            }

            void PrintValueMaxMinAvg(ValueStatistics valueStatistics, string readableName, string unitOfMeasure)
            {
                fileWriter.WriteLine($"  {readableName} max ({unitOfMeasure}): {valueStatistics.MaxValue:0.00}");
                fileWriter.WriteLine($"  {readableName} min ({unitOfMeasure}): {valueStatistics.MinValue:0.00}");
                fileWriter.WriteLine($"  {readableName} avg ({unitOfMeasure}): {valueStatistics.Average:0.00}");
            }

            void PrintValueAvgStdDevMaxMin(ValueStatistics valueStatistics, string readableName, string unitOfMeasure)
            {
                fileWriter.WriteLine($"  {readableName} avg ({unitOfMeasure}): {valueStatistics.Average:0.0}");
                fileWriter.WriteLine($"  {readableName} std dev ({unitOfMeasure}): {valueStatistics.StandardDeviation:0.0}");
                fileWriter.WriteLine($"  {readableName} max ({unitOfMeasure}): {valueStatistics.MaxValue:0.0}");
                fileWriter.WriteLine($"  {readableName} min ({unitOfMeasure}): {valueStatistics.MinValue:0.0}");
            }

            void PrintAvgRate(string readableName, long totalCount, long firstTime, long statsTime)
            {
                if (totalCount <= 0)
                    return;
                var rateValue = totalCount / ((statsTime - firstTime) / 1000000000.0);
                fileWriter.WriteLine($"  Avg {readableName} rate: {rateValue:0}");
            }
        }

        private void CollectGenMsgStats(ProviderThreadStats threadStats, ProviderThread providerThread)
        {
            if (providerThread.ProviderRoleKind == ProviderRoleEnum.INTERACTIVE)
            {
                var latencyRecords = threadStats.GenMsgLatencyRecords;
                TimeRecord? record;
                while ((record = latencyRecords.RecordTracker.GetNextSubmittedTimeRecord()) != null)
                {
                    var latency = (double)(record.EndTime - record.StartTime) / (double)record.Ticks;

                    threadStats.IntervalGenMsgLatencyStats.Update(latency);
                    threadStats.GenMsgLatencyStats.Update(latency);

                    if (config.ThreadCount > 1)
                    {
                        totalStats.GenMsgLatencyStats.Update(latency);
                    }

                    providerThread.LatencyFile.WriteFile($"Gen, {record.StartTime}, {record.EndTime}, {record.EndTime - record.StartTime}\n");

                    latencyRecords.RecordTracker.ReturnTimeRecord(record);
                }
            }
        }

        /// <summary>
        /// Prints the final stats.
        /// </summary>
        private void PrintFinalStats()
        {
            var printWriter = Console.Out;
            PrintSummaryStats(printWriter);
            printWriter.Flush();
            PrintSummaryStats(summaryFile!.Writer!);
        }

        private void Stop()
        {
            StopThreads();
            PrintFinalStats();
            Clear();
        }

        private void RegisterShutdownHook()
        {
            Console.CancelKeyPress += delegate {
                try
                {
                    m_ExitApp = true;
                    m_RunLoopExited.Wait();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }
            };
        }
    }
}
