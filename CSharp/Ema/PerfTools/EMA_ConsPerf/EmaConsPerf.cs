/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.PerfTools.Common;
using System.Net;

namespace LSEG.Ema.PerfTools.ConsPerf
{
    public class EmaConsPerf
    {
#pragma warning disable CS8618
        private const int MAX_CONS_THREADS = 8;

        private PostUserInfo _postUserInfo = new PostUserInfo();

        // array of consumer threads
        private ConsumerThreadInfo[] _consumerThreadsInfo;

        // application configuration information
        private EmaConsPerfConfig _consPerfConfig = new EmaConsPerfConfig();

        // item information list from XML file
        private XmlItemInfoList _xmlItemInfoList;
        private XmlMsgData _xmlMsgData;

        // Total statistics collected from all consumer threads. 
        // This is only used when there are multiple consumer threads. 
        private ConsumerStats _totalStats = new ConsumerStats();

        // CPU & Memory Usage samples 
        private ValueStatistics _cpuUsageStats, _memUsageStats;

        private ResourceUsageStats m_ResourceUsageStats = new ResourceUsageStats();

        // run-time tracking  
        private long _currentTime, _endTime;
        private int _currentRuntimeSec = 0, _intervalSeconds = 0;

        // Logs summary information, such as application inputs and final statistics. 
        private string? _summaryFile = null;
        private StreamWriter? _summaryFileWriter;

        // stdio writer 
        private StreamWriter _stdOutWriter = new StreamWriter(Console.OpenStandardOutput());

#pragma warning restore CS8618

        // timestamp at last stats report 
        private long _previousStatsTime;

        // indicates whether or not application should be Shutdown 
        private volatile bool _shutdownApp = false;

        /** Shutdown ConsPerf */
        public void Shutdown()
        {
            _shutdownApp = true;
        }

        private CountdownEvent _runLoopExited = new CountdownEvent(1);

        /* Run ConsPerf */
        public void Run()
        {
            long nextTime;

            _previousStatsTime = (long)GetTime.GetMicroseconds();

            // Main statistics polling thread here
            while (!_shutdownApp)
            {
                _currentTime = (long)GetTime.GetMicroseconds();
                nextTime = _currentTime + 1000000L;

                if (!(_consPerfConfig.ThreadCount > 0))
                    break;

                if (_intervalSeconds == _consPerfConfig.WriteStatsInterval)
                {
                    CollectStats(true, _consPerfConfig.DisplayStats, _currentRuntimeSec);
                    _intervalSeconds = 0;
                }

                if (_totalStats.ImageRetrievalEndTime > 0 && _consPerfConfig.RequestSnapshots)
                {
                    Console.WriteLine("Received all images for snapshot test.\n");
                    break;
                }

                if (_currentTime >= _endTime)
                {
                    if (_totalStats.ImageRetrievalEndTime == 0)
                        Console.WriteLine("Error: Failed to receive all images within {0:0.0} seconds.\n", _consPerfConfig.SteadyStateTime);

                else
                        Console.WriteLine("\nSteady-state time of {0:0.0} seconds has expired.\n", _consPerfConfig.SteadyStateTime);

                    break;
                }

                try
                {
                    long sleepTime = (nextTime - (long)GetTime.GetMicroseconds()) / 1000;
                    if (sleepTime > 0)
                    {
                        Thread.Sleep((int)sleepTime);
                    }
                    ++_currentRuntimeSec;
                    ++_intervalSeconds;
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Thread.Sleep() failed: {e.Message}");
                    break;
                }
            }

            Stop();
            _runLoopExited.Signal(1);
        }

        private void Stop()
        {
            StopConsumerThreads();

            // print summary
            // If there was only one consumer thread, we didn't bother
            // gathering cross-thread stats, so copy them from the
            // lone consumer.
            if (_consPerfConfig.ThreadCount == 1)
                _totalStats = _consumerThreadsInfo[0].Stats!;

            PrintSummaryStatistics(_stdOutWriter);
            _stdOutWriter.Flush();
            PrintSummaryStatistics(_summaryFileWriter!);
            _summaryFileWriter!.Close();

            for (int i = 0; i < _consPerfConfig.ThreadCount; i++)
            {
                _consumerThreadsInfo[i].Cleanup();
            }
        }

        /* Initializes ConsPerf application. */
        private void Initialize(string[] args)
        {
            int threadCount = 1;

            // initialize and print configuration parameters
            _consPerfConfig.Init(args, MAX_CONS_THREADS);
            Console.WriteLine(_consPerfConfig.ToString());

            // parse item list and message data XML files 
            // the application exits if any error occured
            ParseXmlFiles();

            if (_consPerfConfig.PostsPerSec > 0 && _xmlItemInfoList.PostMsgItemCount == 0)
            {
                Console.WriteLine("Error: Configured for posting but no posting items found in item file.\n");
                Environment.Exit(-1);
            }

            if (_consPerfConfig.GenMsgsPerSec > 0 && _xmlItemInfoList.GenMsgItemCount == 0)
            {
                Console.WriteLine("Error: Configured for sending generic msgs but no generic msg items found in item file.\n");
                Environment.Exit(-1);
            }

            if (_consPerfConfig.ThreadCount > 0)
            {
                threadCount = _consPerfConfig.ThreadCount;
            }

            _consumerThreadsInfo = new ConsumerThreadInfo[threadCount];

            // Calculate unique index for each thread. Each thread requests a common
            // and unique set of items. Unique index is so each thread has a unique
            // index into the shared item list. 
            int itemListUniqueIndex = _consPerfConfig.CommonItemCount;
            for (int i = 0; i < threadCount; ++i)
            {
                _consumerThreadsInfo[i] = new ConsumerThreadInfo();
                // Figure out how many unique items each consumer should request. 
                _consumerThreadsInfo[i].ItemListCount = _consPerfConfig.ItemRequestCount / threadCount;

                /* Distribute remainder. */
                if (i < _consPerfConfig.ItemRequestCount % threadCount)
                    _consumerThreadsInfo[i].ItemListCount = _consumerThreadsInfo[i].ItemListCount + 1;

                _consumerThreadsInfo[i].ItemListUniqueIndex = itemListUniqueIndex;
                itemListUniqueIndex += (_consumerThreadsInfo[i].ItemListCount - _consPerfConfig.CommonItemCount);
            }

            // create summary file writer
            _summaryFile = _consPerfConfig.SummaryFilename!;
            try
            {
                _summaryFileWriter = new StreamWriter(_summaryFile);
            }
            catch
            {
                Console.WriteLine($"Error: Failed to open summary file {_summaryFile}.\n");
                Environment.Exit(-1);
            }

            //write configuration parameters to summary file
            _summaryFileWriter.WriteLine(_consPerfConfig.ToString());
            _summaryFileWriter.Flush();

            //set PostUserInfo information.
            if (_consPerfConfig.PostsPerSec > 0)
            {
                try
                {
                    _postUserInfo.UserId = Thread.CurrentThread.ManagedThreadId;
                    _postUserInfo.UserAddr = BitConverter.ToUInt32(Dns.GetHostAddresses(Dns.GetHostName())
                        .Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)!.FirstOrDefault()?.GetAddressBytes());
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Failed fetching Host Address while setting postUserInfo: {e.Message}\n");
                    Environment.Exit(-1);
                }
            }

            Console.WriteLine("Starting connections(" + _consPerfConfig.ThreadCount + " total)...\n");

            //Spawn consumer threads
            for (int i = 0; i < _consPerfConfig.ThreadCount; ++i)
            {
                _consumerThreadsInfo[i].ThreadId = i + 1;
                new Thread(new ThreadStart(new ConsumerThread(_consumerThreadsInfo[i], _consPerfConfig, _xmlItemInfoList!, _xmlMsgData!, _postUserInfo).Run)).Start();
            }

            // set application end time
            _endTime = (long)GetTime.GetMicroseconds() + _consPerfConfig.SteadyStateTime * 1000000L;

            //create CPU and memory usage statistics
            _cpuUsageStats = new ValueStatistics();
            _memUsageStats = new ValueStatistics();

            // Sleep for one more second so some stats can be gathered before first printout. 
            try
            {
                Thread.Sleep(1000);
            }
            catch
            {
                Console.WriteLine("Thread.Sleep(1000) failed\n");
                Environment.Exit(-1);
            }
        }


        /* Parse item list and message data XML files. */
        private void ParseXmlFiles()
        {
            _xmlItemInfoList = new XmlItemInfoList(_consPerfConfig.ItemRequestCount);
            _xmlMsgData = new XmlMsgData();
            if (_xmlItemInfoList.ParseFile(_consPerfConfig.ItemFilename!) == PerfToolsReturnCode.FAILURE)
            {
                Console.WriteLine($"Failed to load item list from file '{_consPerfConfig.ItemFilename}'.\n");
                Environment.Exit(-1);
            }
            if (_consPerfConfig.PostsPerSec > 0 || _consPerfConfig.GenMsgsPerSec > 0)
            {
                if (_xmlMsgData.ParseFile(_consPerfConfig.MsgFilename!) == PerfToolsReturnCode.FAILURE)
                {
                    Console.WriteLine($"Failed to load message data from file '{_consPerfConfig.MsgFilename}'.\n");
                    Environment.Exit(-1);
                }
            }
        }

        /* Print summary statistics */
        private void PrintSummaryStatistics(StreamWriter fileWriter)
        {
            long firstUpdateTime;
            long totalUpdateCount = _totalStats.StartupUpdateCount.GetTotal()
                + _totalStats.SteadyStateUpdateCount.GetTotal();

            /* Find when the first update was received. */
            firstUpdateTime = 0;
            for (int i = 0; i < _consPerfConfig.ThreadCount; ++i)
            {
                if (firstUpdateTime == 0 || _consumerThreadsInfo[i].Stats!.FirstUpdateTime < firstUpdateTime)
                    firstUpdateTime = _consumerThreadsInfo[i].Stats!.FirstUpdateTime;
            }

            PrintClientSummaryStats(fileWriter);

            fileWriter.Write("\n--- OVERALL SUMMARY ---\n\n");

            fileWriter.Write("Startup State Statistics:\n");

            double duration = _totalStats.ImageRetrievalStartTime > 0
                ? ((_totalStats.ImageRetrievalEndTime > 0 ? _totalStats.ImageRetrievalEndTime : _currentTime) - _totalStats.ImageRetrievalStartTime) / 1000000.0
                : 0.0;
            fileWriter.Write("  Sampling duration (sec): {0:0.000}\n", duration);

            if (_totalStats.StartupLatencyStats.Count > 0)
            {
                fileWriter.Write("  Latency avg (usec): {0:0.0}\n", _totalStats.StartupLatencyStats.Average);
                fileWriter.Write("  Latency std dev (usec): {0:0.0}\n", _totalStats.StartupLatencyStats.StandardDeviation);
                fileWriter.Write("  Latency max (usec): {0:0.0}\n", _totalStats.StartupLatencyStats.MaxValue);
                fileWriter.Write("  Latency min (usec): {0:0.0}\n", _totalStats.StartupLatencyStats.MinValue);
            }
            else
                fileWriter.Write("  No latency information received during startup time.\n\n");

            double updRate = _totalStats.StartupUpdateCount.GetTotal() / (((_totalStats.ImageRetrievalEndTime > 0 ? _totalStats.ImageRetrievalEndTime : _currentTime) - firstUpdateTime) / 1000000.0);
            fileWriter.Write("  Avg update rate: {0:0.0}\n\n", updRate);

            fileWriter.Write("Steady State Statistics:\n");

            if (_totalStats.ImageRetrievalEndTime > 0)
            {

                fileWriter.Write("  Sampling duration (sec): {0:0.000}\n", (_currentTime - _totalStats.ImageRetrievalEndTime) / 1000000.0);

                if (_totalStats.SteadyStateLatencyStats.Count > 0)
                {
                    fileWriter.Write("  Latency avg (usec): {0:0.0}\n", _totalStats.SteadyStateLatencyStats.Average);
                    fileWriter.Write("  Latency std dev (usec): {0:0.0}\n", _totalStats.SteadyStateLatencyStats.StandardDeviation);
                    fileWriter.Write("  Latency max (usec): {0:0.0}\n", _totalStats.SteadyStateLatencyStats.MaxValue);
                    fileWriter.Write("  Latency min (usec): {0:0.0}\n", _totalStats.SteadyStateLatencyStats.MinValue);
                }
                else
                    fileWriter.Write("  No latency information was received during steady-state time.\n");

                if (_consPerfConfig.LatencyPostsPerSec > 0)
                {
                    if (_totalStats.PostLatencyStats.Count > 0)
                    {
                        fileWriter.Write("  Posting latency avg (usec): {0:0.0}\n", _totalStats.PostLatencyStats.Average);
                        fileWriter.Write("  Posting latency std dev (usec): {0:0.0}\n", _totalStats.PostLatencyStats.StandardDeviation);
                        fileWriter.Write("  Posting latency max (usec): {0:0.0}\n", _totalStats.PostLatencyStats.MaxValue);
                        fileWriter.Write("  Posting latency min (usec): {0:0.0}\n", _totalStats.PostLatencyStats.MinValue);
                    }
                    else
                        fileWriter.Write("  No posting latency information was received during steady-state time.\n");
                }

                fileWriter.Write("  Avg update rate: {0:0.0}\n",
                        _totalStats.SteadyStateUpdateCount.GetTotal()
                        / ((_currentTime - _totalStats.ImageRetrievalEndTime) / 1000000.0));

                fileWriter.Write("\n");
            }
            else
                fileWriter.Write("  Steady state was not reached during this test.\n\n");

            fileWriter.Write("Overall Statistics: \n");

            fileWriter.Write("  Sampling duration (sec): {0:0.000}\n", 
                (_totalStats.ImageRetrievalStartTime > 0) ? ((double)_currentTime - (double)_totalStats.ImageRetrievalStartTime) / 1000000.0 : 0.0);

            if (_totalStats.OverallLatencyStats.Count > 0)
            {
                fileWriter.Write("  Latency avg (usec): {0:0.0}\n", _totalStats.OverallLatencyStats.Average);
                fileWriter.Write("  Latency std dev (usec): {0:0.0}\n", _totalStats.OverallLatencyStats.StandardDeviation);
                fileWriter.Write("  Latency max (usec): {0:0.0}\n", _totalStats.OverallLatencyStats.MaxValue);
                fileWriter.Write("  Latency min (usec): {0:0.0}\n", _totalStats.OverallLatencyStats.MinValue);
            }
            else
                fileWriter.Write("  No latency information was received.\n");

            if (_totalStats.GenMsgLatencyStats.Count > 0)
            {
                fileWriter.Write("  GenMsg latency avg (usec): {0:0.0}\n", _totalStats.GenMsgLatencyStats.Average);
                fileWriter.Write("  GenMsg latency std dev (usec): {0:0.0}\n", _totalStats.GenMsgLatencyStats.StandardDeviation);
                fileWriter.Write("  GenMsg latency max (usec): {0:0.0}\n", _totalStats.GenMsgLatencyStats.MaxValue);
                fileWriter.Write("  GenMsg latency min (usec): {0:0.0}\n", _totalStats.GenMsgLatencyStats.MinValue);
            }
            else
                fileWriter.Write("  No GenMsg latency information was received.\n");

            if (_cpuUsageStats.Count > 0)
            {
                fileWriter.Write("  CPU/Memory Samples: {0}\n", _cpuUsageStats.Count);
                fileWriter.Write("  CPU Usage max (%%): {0:0.00}\n", _cpuUsageStats.MaxValue);
                fileWriter.Write("  CPU Usage min (%%): {0:0.00}\n", _cpuUsageStats.MinValue);
                fileWriter.Write("  CPU Usage avg (%%): {0:0.00}\n", _cpuUsageStats.Average);
                fileWriter.Write("  Memory Usage max (MB): {0:0.00}\n", _memUsageStats.MaxValue);
                fileWriter.Write("  Memory Usage min (MB): {0:0.00}\n", _memUsageStats.MinValue);
                fileWriter.Write("  Memory Usage avg (MB): {0:0.00}\n", _memUsageStats.Average);
            }

            fileWriter.Write("\nTest Statistics:\n");

            fileWriter.Write("  Requests sent: {0}\n", _totalStats.RequestCount.GetTotal());
            fileWriter.Write("  Refreshes received: {0}\n", _totalStats.RefreshCount.GetTotal());
            fileWriter.Write("  Updates received: {0}\n", totalUpdateCount);

            if (_consPerfConfig.PostsPerSec > 0)
            {
                fileWriter.Write("  Posts sent: {0}\n", _totalStats.PostSentCount.GetTotal());
            }

            if (_consPerfConfig.GenMsgsPerSec > 0)
                fileWriter.Write("  GenMsgs sent: {0}\n", _totalStats.GenMsgSentCount.GetTotal());
            if (_totalStats.GenMsgRecvCount.GetTotal() > 0)
                fileWriter.Write("  GenMsgs received: {0}\n", _totalStats.GenMsgRecvCount.GetTotal());
            if (_consPerfConfig.LatencyGenMsgsPerSec > 0)
                fileWriter.Write("  GenMsg latencies sent: {0}\n", _totalStats.LatencyGenMsgSentCount.GetTotal());
            if (_totalStats.GenMsgLatencyStats.Count > 0)
                fileWriter.Write("  GenMsg latencies received: {0}\n", _totalStats.GenMsgLatencyStats.Count);

            if (_totalStats.ImageRetrievalEndTime > 0)
            {
                long totalRefreshRetrievalTime = (_totalStats.ImageRetrievalEndTime -
                        _totalStats.ImageRetrievalStartTime);

                fileWriter.Write("  Image retrieval time (sec): {0:0.000}\n", totalRefreshRetrievalTime / 1000000.0);
                fileWriter.Write("  Avg image Rate: {0:0.0}\n", _consPerfConfig.ItemRequestCount / (totalRefreshRetrievalTime / 1000000.0));
            }

            fileWriter.Write("  Avg update rate: {0:0.0}\n",  totalUpdateCount / ((_currentTime - firstUpdateTime) / 1000000.0));

            if (_totalStats.PostSentCount.GetTotal() > 0)
            {
                fileWriter.Write("  Avg posting rate: {0:0.0}\n", _totalStats.PostSentCount.GetTotal() / ((_currentTime - _totalStats.ImageRetrievalEndTime) / 1000000.0));
            }

            if (_consPerfConfig.GenMsgsPerSec > 0)
            {
                fileWriter.Write("  Avg GenMsg send rate: {0:0.0}\n", _totalStats.GenMsgSentCount.GetTotal() / ((_currentTime - _totalStats.ImageRetrievalEndTime) / 1000000.0));
            }
            if (_totalStats.GenMsgRecvCount.GetTotal() > 0)
            {
                fileWriter.Write("  Avg GenMsg receive rate: {0:0.0}\n", _totalStats.GenMsgRecvCount.GetTotal() / ((_currentTime - _totalStats.ImageRetrievalEndTime) / 1000000.0));
            }
            if (_consPerfConfig.LatencyGenMsgsPerSec > 0)
            {
                fileWriter.Write("  Avg GenMsg latency send rate: {0:0.0}\n", _totalStats.LatencyGenMsgSentCount.GetTotal() / ((_currentTime - _totalStats.ImageRetrievalEndTime) / 1000000.0));
            }
            if (_totalStats.GenMsgLatencyStats.Count > 0)
            {
                fileWriter.Write("  Avg GenMsg latency receive rate: {0:0.0}\n", _totalStats.GenMsgLatencyStats.Count / ((_currentTime - _totalStats.ImageRetrievalEndTime) / 1000000.0));
            }

            fileWriter.Write("\n");
        }

        /* Print client summary statistics. */
        private void PrintClientSummaryStats(StreamWriter fileWriter)
        {
            for (int i = 0; i < _consPerfConfig.ThreadCount; ++i)
            {
                long imageRetrievalTime = (_consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime > 0) 
                    ? (_consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime - _consumerThreadsInfo[i].Stats!.ImageRetrievalStartTime) 
                    : 0;

                /* If there are multiple connections, print individual summaries. */
                if (_consPerfConfig.ThreadCount > 1)
                {
                    long totalClientUpdateCount =
                        _consumerThreadsInfo[i].Stats!.StartupUpdateCount.GetTotal()
                        + _consumerThreadsInfo[i].Stats!.SteadyStateUpdateCount.GetTotal();

                    fileWriter.Write($"\n--- CLIENT {i + 1} SUMMARY ---\n\n");

                    fileWriter.Write("Startup State Statistics:\n");

                    fileWriter.Write("  Sampling duration (sec): {0:0.000}\n",
                            (_consumerThreadsInfo[i].Stats!.ImageRetrievalStartTime > 0) ?
                            (((double)((_consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime > 0) ? _consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime : _currentTime)
                             - (double)_consumerThreadsInfo[i].Stats!.ImageRetrievalStartTime) / 1000000.0) : 0.0);

                    if (_consumerThreadsInfo[i].Stats!.StartupLatencyStats.Count > 0)
                    {
                        fileWriter.Write("  Latency avg (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.StartupLatencyStats.Average);
                        fileWriter.Write("  Latency std dev (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.StartupLatencyStats.StandardDeviation);
                        fileWriter.Write("  Latency max (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.StartupLatencyStats.MaxValue);
                        fileWriter.Write("  Latency min (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.StartupLatencyStats.MinValue);
                    }
                    else
                        fileWriter.Write("  No latency information received during startup time.\n\n");

                    fileWriter.Write("  Avg update rate: {0:0.0}\n",
                            _consumerThreadsInfo[i].Stats!.StartupUpdateCount.GetTotal() /
                            ((
                                    ((_consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime > 0) ?
                                    _consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime : _currentTime)
                                    - _consumerThreadsInfo[i].Stats!.FirstUpdateTime
                                    ) / 1000000.0));

                    fileWriter.Write("\nSteady State Statistics:\n");

                    if (_consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime > 0)
                    {
                        fileWriter.Write("  Sampling duration (sec): {0:0.000}\n",
                                ((double)_currentTime - (double)_consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime) / 1000000.0);
                        if (_consumerThreadsInfo[i].Stats!.SteadyStateLatencyStats.Count > 0)
                        {
                            fileWriter.Write("  Latency avg (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.SteadyStateLatencyStats.Average);
                            fileWriter.Write("  Latency std dev (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.SteadyStateLatencyStats.StandardDeviation);
                            fileWriter.Write("  Latency max (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.SteadyStateLatencyStats.MaxValue);
                            fileWriter.Write("  Latency min (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.SteadyStateLatencyStats.MinValue);
                        }
                        else
                            fileWriter.Write("  No latency information was received during steady-state time.\n");

                        if (_consPerfConfig.LatencyPostsPerSec > 0)
                        {
                            if (_consumerThreadsInfo[i].Stats!.PostLatencyStats.Count > 0)
                            {
                                fileWriter.Write("  Posting latency avg (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.PostLatencyStats.Average);
                                fileWriter.Write("  Posting latency std dev (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.PostLatencyStats.StandardDeviation);
                                fileWriter.Write("  Posting latency max (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.PostLatencyStats.MaxValue);
                                fileWriter.Write("  Posting latency min (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.PostLatencyStats.MinValue);
                            }
                            else
                                fileWriter.Write("  No posting latency information was received during steady-state time.\n");
                        }


                        fileWriter.Write("  Avg update rate: {0:0.0}\n",
                                _consumerThreadsInfo[i].Stats!.SteadyStateUpdateCount.GetTotal()
                                / ((_currentTime - _consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime) / 1000000.0));
                    }
                    else
                        fileWriter.Write("  Steady state was not reached during this test.\n\n");

                    fileWriter.Write("\nOverall Statistics: \n");

                    fileWriter.Write(
                            "  Sampling duration (sec): {0:0.000}\n",
                            (_consumerThreadsInfo[i].Stats!.ImageRetrievalStartTime > 0) ?
                            ((double)_currentTime
                             - (double)_consumerThreadsInfo[i].Stats!.ImageRetrievalStartTime) / 1000000.0 : 0.0);

                    if (_consumerThreadsInfo[i].Stats!.OverallLatencyStats.Count > 0)
                    {
                        fileWriter.Write("  Latency avg (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.OverallLatencyStats.Average);
                        fileWriter.Write("  Latency std dev (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.OverallLatencyStats.StandardDeviation);
                        fileWriter.Write("  Latency max (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.OverallLatencyStats.MaxValue);
                        fileWriter.Write("  Latency min (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.OverallLatencyStats.MinValue);
                    }
                    else
                        fileWriter.Write("  No latency information was received.\n");

                    if (_consPerfConfig.LatencyGenMsgsPerSec > 0)
                    {
                        if (_consumerThreadsInfo[i].Stats!.GenMsgLatencyStats.Count > 0)
                        {
                            fileWriter.Write("  GenMsg latency avg (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.GenMsgLatencyStats.Average);
                            fileWriter.Write("  GenMsg latency std dev (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.GenMsgLatencyStats.StandardDeviation);
                            fileWriter.Write("  GenMsg latency max (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.GenMsgLatencyStats.MaxValue);
                            fileWriter.Write("  GenMsg latency min (usec): {0:0.0}\n", _consumerThreadsInfo[i].Stats!.GenMsgLatencyStats.MinValue);
                        }
                        else
                            fileWriter.Write("  No GenMsg latency information was received.\n");
                    }

                    fileWriter.Write("\nTest Statistics:\n");

                    fileWriter.Write($"  Requests sent: {_consumerThreadsInfo[i].Stats!.RequestCount.GetTotal()}\n");
                    fileWriter.Write($"  Refreshes received: {_consumerThreadsInfo[i].Stats!.RefreshCount.GetTotal()}\n");
                    fileWriter.Write($"  Updates received: {totalClientUpdateCount}\n");

                    if (_consPerfConfig.PostsPerSec > 0)
                    {
                        fileWriter.Write($"  Posts sent: {_consumerThreadsInfo[i].Stats!.PostSentCount.GetTotal()}\n");
                    }

                    if (_consPerfConfig.GenMsgsPerSec > 0)
                        fileWriter.Write($"  GenMsgs sent: {_consumerThreadsInfo[i].Stats!.GenMsgSentCount.GetTotal()}\n");
                    if (_consumerThreadsInfo[i].Stats!.GenMsgRecvCount.GetTotal() > 0)
                        fileWriter.Write($"  GenMsgs received: { _consumerThreadsInfo[i].Stats!.GenMsgRecvCount.GetTotal()}\n");
                    if (_consPerfConfig.LatencyGenMsgsPerSec > 0)
                        fileWriter.Write($"  GenMsg latencies sent: {_consumerThreadsInfo[i].Stats!.LatencyGenMsgSentCount.GetTotal()}\n");
                    if (_consumerThreadsInfo[i].Stats!.GenMsgLatencyStats.Count > 0)
                        fileWriter.Write($"  GenMsg latencies received: {_consumerThreadsInfo[i].Stats!.GenMsgLatencyStats.Count}\n");

                    if (imageRetrievalTime > 0)
                    {
                        fileWriter.Write("  Image retrieval time(sec): {0:0.000}\n", imageRetrievalTime / 1000000.0);
                        fileWriter.Write("  Avg image Rate: {0:0.0}\n", _consumerThreadsInfo[i].Stats!.RefreshCount.GetTotal() / (imageRetrievalTime / 1000000.0));
                    }

                    fileWriter.Write("  Avg update rate: {0:0.0}\n",  totalClientUpdateCount / ((_currentTime - _consumerThreadsInfo[i].Stats!.FirstUpdateTime) / 1000000.0));

                    if (_consPerfConfig.PostsPerSec > 0)
                    {
                        fileWriter.Write("  Avg posting rate: {0:0.0}\n",
                                _consumerThreadsInfo[i].Stats!.PostSentCount.GetTotal() / ((_currentTime - _consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime) / 1000000.0));
                    }

                    if (_consPerfConfig.GenMsgsPerSec > 0)
                    {
                        fileWriter.Write("  Avg GenMsg send rate: {0:0.0}\n", _consumerThreadsInfo[i].Stats!.GenMsgSentCount.GetTotal() / ((_currentTime - _consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime) / 1000000.0));
                    }
                    if (_consumerThreadsInfo[i].Stats!.GenMsgRecvCount.GetTotal() > 0)
                    {
                        fileWriter.Write("  Avg GenMsg receive rate: {0:0.0}\n", _consumerThreadsInfo[i].Stats!.GenMsgRecvCount.GetTotal() / ((_currentTime - _consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime) / 1000000.0));
                    }
                    if (_consPerfConfig.LatencyGenMsgsPerSec > 0)
                    {
                        fileWriter.Write("  Avg GenMsg latency send rate: {0:0.0}\n", _consumerThreadsInfo[i].Stats!.LatencyGenMsgSentCount.GetTotal() / ((_currentTime - _consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime) / 1000000.0));
                    }
                    if (_consumerThreadsInfo[i].Stats!.GenMsgLatencyStats.Count > 0)
                    {
                        fileWriter.Write("  Avg GenMsg latency receive rate: {0:0.0}\n", _consumerThreadsInfo[i].Stats!.GenMsgLatencyStats.Count / ((_currentTime - _consumerThreadsInfo[i].Stats!.ImageRetrievalEndTime) / 1000000.0));
                    }
                }
            }
        }

        /* Collect statistics. */
        private void CollectStats(bool writeStats, bool displayStats, int currentRuntimeSec)
        {
            bool allRefreshesRetrieved = true;

            long timeNow = (long)GetTime.GetMicroseconds();
            long statsInterval = timeNow - _previousStatsTime;
            _previousStatsTime = timeNow;

            m_ResourceUsageStats.Refresh();
            double processCpuLoad = m_ResourceUsageStats.CurrentProcessCpuLoad();
            double memoryUsage = m_ResourceUsageStats.CurrentMemoryUsage();

            _cpuUsageStats.Update(processCpuLoad);
            _memUsageStats.Update(memoryUsage);

            for (int i = 0; i < _consPerfConfig.ThreadCount; i++)
            {
                long refreshCount,
                    startupUpdateCount,
                    steadyStateUpdateCount,
                    requestCount,
                    statusCount,
                    postOutOfBuffersCount,
                    postSentCount,
                    genMsgSentCount,
                    genMsgRecvCount,
                    latencyGenMsgSentCount,
                    latencyGenMsgRecvCount,
                    genMsgOutOfBuffersCount;


                /* Gather latency records from each thread and update statistics. */
                CollectUpdateStats(_consumerThreadsInfo[i]);

                /* Gather latency records for posts. */
                CollectPostStats(_consumerThreadsInfo[i]);

                /* Gather latency records for generic msgs. */
                CollectGenMsgStats(_consumerThreadsInfo[i]);

                if (_consumerThreadsInfo[i].LatencyLogFile != null)
                    _consumerThreadsInfo[i].LatencyLogFileWriter!.Flush();

                /* Collect counts. */
                startupUpdateCount = _consumerThreadsInfo[i].Stats!.StartupUpdateCount.GetChange();
                steadyStateUpdateCount = _consumerThreadsInfo[i].Stats!.SteadyStateUpdateCount.GetChange();
                statusCount = _consumerThreadsInfo[i].Stats!.StatusCount.GetChange();
                requestCount = _consumerThreadsInfo[i].Stats!.RequestCount.GetChange();
                refreshCount = _consumerThreadsInfo[i].Stats!.RefreshCount.GetChange();
                postOutOfBuffersCount = _consumerThreadsInfo[i].Stats!.GenMsgOutOfBuffersCount.GetChange();
                postSentCount = _consumerThreadsInfo[i].Stats!.PostSentCount.GetChange();
                genMsgSentCount = _consumerThreadsInfo[i].Stats!.GenMsgSentCount.GetChange();
                genMsgRecvCount = _consumerThreadsInfo[i].Stats!.GenMsgRecvCount.GetChange();
                latencyGenMsgSentCount = _consumerThreadsInfo[i].Stats!.LatencyGenMsgSentCount.GetChange();
                latencyGenMsgRecvCount = _consumerThreadsInfo[i].Stats!.IntervalGenMsgLatencyStats.Count;
                genMsgOutOfBuffersCount = _consumerThreadsInfo[i].Stats!.PostOutOfBuffersCount.GetChange();

                if (_consPerfConfig.ThreadCount > 1)
                {
                    _totalStats.StartupUpdateCount.Add(startupUpdateCount);
                    _totalStats.SteadyStateUpdateCount.Add(steadyStateUpdateCount);
                    _totalStats.StatusCount.Add(statusCount);
                    _totalStats.RequestCount.Add(requestCount);
                    _totalStats.RefreshCount.Add(refreshCount);
                    _totalStats.PostOutOfBuffersCount.Add(postOutOfBuffersCount);
                    _totalStats.PostSentCount.Add(postSentCount);
                    _totalStats.GenMsgSentCount.Add(genMsgSentCount);
                    _totalStats.GenMsgRecvCount.Add(genMsgRecvCount);
                    _totalStats.LatencyGenMsgSentCount.Add(latencyGenMsgSentCount);
                    _totalStats.GenMsgOutOfBuffersCount.Add(genMsgOutOfBuffersCount);
                }

                if (writeStats)
                {
                    /* Log statistics to file. */
                    PrintCurrentTimeUTC(_consumerThreadsInfo[i].StatsFileWriter!);
                    _consumerThreadsInfo[i].StatsFileWriter!.Write(
                            ", {0}, {1:0.0}, {2:0.0}, {3:0.0}, {4:0.0}, {5}, {6}, {7}, {8:0.0}, {9:0.0}, {10:0.0}, {11:0.0}, {12}, {13}, {14}, {15}, {16:0.0}, {17:0.0}, {18:0.0}, {19:0.0}, {20:0.00}, {21:0.00}\n",
                            _consumerThreadsInfo[i].Stats!.IntervalLatencyStats.Count,
                            _consumerThreadsInfo[i].Stats!.IntervalLatencyStats.Average,
                            _consumerThreadsInfo[i].Stats!.IntervalLatencyStats.StandardDeviation,
                            (_consumerThreadsInfo[i].Stats!.IntervalLatencyStats.Count > 0) ? _consumerThreadsInfo[i].Stats!.IntervalLatencyStats.MaxValue : 0.0,
                            (_consumerThreadsInfo[i].Stats!.IntervalLatencyStats.Count > 0) ? _consumerThreadsInfo[i].Stats!.IntervalLatencyStats.MinValue : 0.0,
                            refreshCount,
                            (startupUpdateCount + steadyStateUpdateCount) * 1000000L / statsInterval,
                            _consumerThreadsInfo[i].Stats!.IntervalPostLatencyStats.Count,
                            _consumerThreadsInfo[i].Stats!.IntervalPostLatencyStats.Average,
                            _consumerThreadsInfo[i].Stats!.IntervalPostLatencyStats.StandardDeviation,
                            (_consumerThreadsInfo[i].Stats!.IntervalPostLatencyStats.Count > 0) ? _consumerThreadsInfo[i].Stats!.IntervalPostLatencyStats.MaxValue : 0.0,
                            (_consumerThreadsInfo[i].Stats!.IntervalPostLatencyStats.Count > 0) ? _consumerThreadsInfo[i].Stats!.IntervalPostLatencyStats.MinValue : 0.0,
                            genMsgSentCount,
                            genMsgRecvCount,
                            latencyGenMsgSentCount,
                            latencyGenMsgRecvCount,
                            _consumerThreadsInfo[i].Stats!.IntervalGenMsgLatencyStats.Average,
                            _consumerThreadsInfo[i].Stats!.IntervalGenMsgLatencyStats.StandardDeviation,
                            (_consumerThreadsInfo[i].Stats!.IntervalGenMsgLatencyStats.Count > 0) ? _consumerThreadsInfo[i].Stats!.IntervalGenMsgLatencyStats.MaxValue : 0.0,
                                    (_consumerThreadsInfo[i].Stats!.IntervalGenMsgLatencyStats.Count > 0) ? _consumerThreadsInfo[i].Stats!.IntervalGenMsgLatencyStats.MinValue : 0.0,
                            processCpuLoad,
                            memoryUsage);
                    _consumerThreadsInfo[i].StatsFileWriter!.Flush();
                }

                if (displayStats)
                {
                    if (_consPerfConfig.ThreadCount == 1)
                        Console.WriteLine("{0}: ", currentRuntimeSec);

                else
                        Console.WriteLine("{0}: Client {1}:\n  ", currentRuntimeSec, i + 1);

                    Console.WriteLine("Images: {0}, Posts: {1}, UpdRate: {2}, CPU: {3:0.00}, Mem: {4:000.00}MB\n",
                            refreshCount,
                            postSentCount,
                            (startupUpdateCount + steadyStateUpdateCount) * 1000000L / statsInterval,
                            processCpuLoad,
                            memoryUsage);

                    if (_consumerThreadsInfo[i].Stats!.IntervalLatencyStats.Count > 0)
                    {
                        _consumerThreadsInfo[i].Stats!.IntervalLatencyStats.Print("  Latency(usec)", "Msgs", false);
                        _consumerThreadsInfo[i].Stats!.IntervalLatencyStats.Clear();
                    }

                    if (postOutOfBuffersCount > 0)
                        Console.WriteLine("  - {0} posts not sent due to lack of output buffers.\n", postOutOfBuffersCount);


                    if (_consumerThreadsInfo[i].Stats!.IntervalPostLatencyStats.Count > 0)
                    {
                        _consumerThreadsInfo[i].Stats!.IntervalPostLatencyStats.Print("  PostLat(usec)", "Msgs", false);
                        _consumerThreadsInfo[i].Stats!.IntervalPostLatencyStats.Clear();
                    }

                    if (genMsgSentCount > 0 || genMsgRecvCount > 0)
                        Console.WriteLine("  GenMsgs: sent {0}, received {1}, latencies sent {2}, latencies received {3}\n", genMsgSentCount, genMsgRecvCount, latencyGenMsgSentCount, latencyGenMsgRecvCount);

                    if (genMsgOutOfBuffersCount > 0)
                        Console.WriteLine("  - {0} GenMsgs not sent due to lack of output buffers.\n", genMsgOutOfBuffersCount);

                    if (_consumerThreadsInfo[i].Stats!.IntervalGenMsgLatencyStats.Count > 0)
                    {
                        _consumerThreadsInfo[i].Stats!.IntervalGenMsgLatencyStats.Print("  GenMsgLat(usec)", "Msgs", false);
                        _consumerThreadsInfo[i].Stats!.IntervalGenMsgLatencyStats.Clear();
                    }

                    if (statusCount > 0)
                        Console.WriteLine("  - Received {0} status messages.\n", statusCount);
                }

                /* Get Image Retrieval time for this client. */
                if (GetClientImageRetrievalTime(_consumerThreadsInfo[i], displayStats) == false)
                {
                    allRefreshesRetrieved = false;
                }
            }

            if (!_totalStats.ImageTimeRecorded && allRefreshesRetrieved && _consPerfConfig.ThreadCount > 0)
            {
                _endTime = _totalStats.ImageRetrievalEndTime + _consPerfConfig.SteadyStateTime * 1000000L;
                _totalStats.ImageTimeRecorded = true;

                if (_consPerfConfig.ThreadCount > 1)
                {
                    if (displayStats)
                    {
                        /* Print overall image retrieval stats. */
                        long totalRefreshRetrievalTime = (_totalStats.ImageRetrievalEndTime -
                                _totalStats.ImageRetrievalStartTime);

                        Console.WriteLine("\nOverall image retrieval time for {0} images: {1:0.000}s ({2:0.0} Images/s).\n\n",
                                _consPerfConfig.ItemRequestCount,
                                totalRefreshRetrievalTime / 1000000.0,
                                _consPerfConfig.ItemRequestCount /
                                (totalRefreshRetrievalTime / 1000000.0)
                              );
                    }
                }
                else
                {
                    _totalStats.ImageRetrievalStartTime = _consumerThreadsInfo[0].Stats!.ImageRetrievalStartTime;
                    _totalStats.ImageRetrievalEndTime = _consumerThreadsInfo[0].Stats!.ImageRetrievalEndTime;
                    _totalStats.SteadyStateLatencyTime = _totalStats.ImageRetrievalEndTime + _consPerfConfig.DelaySteadyStateCalc * 1000000L;
                }
            }
        }

        /// <summary>
        /// Collect update statistics.
        /// </summary>
        private void CollectUpdateStats(ConsumerThreadInfo consumerThread)
        {
            TimeRecordQueue latencyRecords = consumerThread.LatencyRecords;

            TimeRecord? record = latencyRecords.RecordTracker.GetNextSubmittedTimeRecord();

            while (record != null)
            {
                double latency = (double)(record.EndTime - record.StartTime) / (double)record.Ticks;
                double recordEndTimeNsec = (double)record.EndTime / (double)record.Ticks * 1000.0;

                /* Make sure this latency is counted towards startup or steady-state as appropriate. */
                bool latencyIsSteadyStateForClient =
                    consumerThread.Stats!.ImageRetrievalEndTime != 0
                    && recordEndTimeNsec > consumerThread.Stats!.ImageRetrievalEndTime;

                consumerThread.Stats!.IntervalLatencyStats.Update(latency);
                consumerThread.Stats!.OverallLatencyStats.Update(latency);
                if (latencyIsSteadyStateForClient)
                {
                    if (recordEndTimeNsec > consumerThread.Stats!.SteadyStateLatencyTime)
                    {
                        consumerThread.Stats!.SteadyStateLatencyStats.Update(latency);
                    }
                }
                else
                {
                    consumerThread.Stats!.StartupLatencyStats.Update(latency);
                }

                if (_consPerfConfig.ThreadCount > 1)
                {
                    /* Make sure this latency is counted towards startup or steady-state as appropriate. */
                    bool latencyIsSteadyStateOverall =
                        _totalStats.ImageRetrievalEndTime != 0
                        && recordEndTimeNsec > _totalStats.ImageRetrievalEndTime;

                    if (latencyIsSteadyStateOverall)
                    {
                        if (recordEndTimeNsec > _totalStats.SteadyStateLatencyTime)
                        {
                            _totalStats.SteadyStateLatencyStats.Update(latency);
                        }
                    }
                    else
                    {
                        _totalStats.StartupLatencyStats.Update(latency);
                    }
                    _totalStats.OverallLatencyStats.Update(latency);
                }

                if (consumerThread.LatencyLogFile != null)
                    consumerThread.LatencyLogFileWriter!.Write($"Upd, {record.StartTime}, {record.EndTime}, {record.EndTime - record.StartTime}\n");

                latencyRecords.RecordTracker.ReturnTimeRecord(record);
                record = latencyRecords.RecordTracker.GetNextSubmittedTimeRecord();
            }
        }

        /// <summary>
        /// Collect post statistics.
        /// </summary>
        private void CollectPostStats(ConsumerThreadInfo consumerThread)
        {
            TimeRecordQueue latencyRecords = consumerThread.PostLatencyRecords;
            TimeRecord? record = latencyRecords.RecordTracker.GetNextSubmittedTimeRecord();
            while (record != null)
            {
                double latency = (record.EndTime - record.StartTime) / (double)record.Ticks;

                consumerThread.Stats!.IntervalPostLatencyStats.Update(latency);
                consumerThread.Stats!.PostLatencyStats.Update(latency);

                if (_consPerfConfig.ThreadCount > 1)
                    _totalStats.PostLatencyStats.Update(latency);

                if (consumerThread.LatencyLogFile != null)
                    consumerThread.LatencyLogFileWriter!.Write($"Pst, {record.StartTime}, {record.EndTime}, {(record.EndTime - record.StartTime)}\n");

                latencyRecords.RecordTracker.ReturnTimeRecord(record);
                record = latencyRecords.RecordTracker.GetNextSubmittedTimeRecord();
            }
        }

        /// <summary>
        /// Collect generic message statistics.
        /// </summary>
        private void CollectGenMsgStats(ConsumerThreadInfo consumerThread)
        {
            TimeRecordQueue latencyRecords = consumerThread.GenMsgLatencyRecords;
            var record = latencyRecords.RecordTracker.GetNextSubmittedTimeRecord();
            while (record != null)
            {
                double latency = (record.EndTime - record.StartTime) / (double)record.Ticks;

                consumerThread.Stats!.IntervalGenMsgLatencyStats.Update(latency);
                consumerThread.Stats!.GenMsgLatencyStats.Update(latency);

                if (_consPerfConfig.ThreadCount > 1)
                    _totalStats.GenMsgLatencyStats.Update(latency);

                if (consumerThread.LatencyLogFile != null)
                    consumerThread.LatencyLogFileWriter!.Write($"Gen, {record.StartTime}, {record.EndTime}, {record.EndTime - record.StartTime}\n");

                latencyRecords.RecordTracker.ReturnTimeRecord(record);
                record = latencyRecords.RecordTracker.GetNextSubmittedTimeRecord();
            }
        }

        /// <summary>
        /// Get client image retrieval time.
        /// </summary>
        private bool GetClientImageRetrievalTime(ConsumerThreadInfo consumerThread, bool displayStats)
        {
            bool allRefreshesRetrieved = true;

            if (!_totalStats.ImageTimeRecorded)
            {
                if (consumerThread.Stats!.ImageRetrievalEndTime > 0)
                {
                    long imageRetrievalStartTime = consumerThread.Stats!.ImageRetrievalStartTime;
                    long imageRetrievalEndTime = consumerThread.Stats!.ImageRetrievalEndTime;

                    /* To get the total time it took to retrieve all images, find the earliest start time
                     * and latest end time across all connections. */
                    if (_totalStats.ImageRetrievalStartTime == 0 ||
                            imageRetrievalStartTime < _totalStats.ImageRetrievalStartTime)
                        _totalStats.ImageRetrievalStartTime = imageRetrievalStartTime;
                    if (_totalStats.ImageRetrievalEndTime == 0 ||
                            imageRetrievalEndTime > _totalStats.ImageRetrievalEndTime)
                    {
                        _totalStats.ImageRetrievalEndTime = imageRetrievalEndTime;
                        _totalStats.SteadyStateLatencyTime = imageRetrievalEndTime + _consPerfConfig.DelaySteadyStateCalc * 1000000L;
                    }
                }
                /* Ignore connections that don't request anything. */
                else if (consumerThread.ItemListCount > 0)
                {
                    allRefreshesRetrieved = false; /* Not all connections have received their images yet. */
                    _totalStats.ImageRetrievalStartTime = 0;
                    _totalStats.ImageRetrievalEndTime = 0;
                }
            }

            if (!consumerThread.Stats!.ImageTimeRecorded && consumerThread.Stats!.ImageRetrievalEndTime > 0)
            {
                consumerThread.Stats!.ImageTimeRecorded = true;

                if (displayStats)
                {
                    long imageRetrievalTime = consumerThread.Stats!.ImageRetrievalEndTime -
                        consumerThread.Stats!.ImageRetrievalStartTime;

                    Console.WriteLine("  - Image retrieval time for {0} images: {1:0.000}s ({2:0.0} images/s)\n",
                            consumerThread.ItemListCount,
                            imageRetrievalTime / 1000000.0,
                            consumerThread.ItemListCount /
                            (imageRetrievalTime / 1000000.0));
                }
            }

            return allRefreshesRetrieved;
        }

        /// <summary>
        /// Stop all consumer threads.
        /// </summary>
        private void StopConsumerThreads()
        {
            for (int i = 0; i < _consPerfConfig.ThreadCount; i++)
            {
                _consumerThreadsInfo[i].Shutdown = true;
            }

            for (int i = 0; i < _consPerfConfig.ThreadCount; i++)
            {
                int count = 0;
                // wait for consumer thread cleanup or timeout
                while (!_consumerThreadsInfo[i].ShutdownAck && count < 3)
                {
                    try
                    {
                        Thread.Sleep(1000);
                        count++;
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine($"Thread.Sleep(1000) failed: {e.Message}");
                    }
                }
            }
        }

        private void PrintCurrentTimeUTC(StreamWriter fileWriter)
        {
            var nowTime = System.DateTime.UtcNow;
            fileWriter.Write(nowTime.ToString());
        }

        private void RegisterShutdownDelegate()
        {
            Console.CancelKeyPress += delegate {
                try
                {
                    Shutdown();
                    _runLoopExited.Wait();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.StackTrace);
                }
            };
        }

        public static void Main(string[] args)
        {
            EmaConsPerf consumerperf = new EmaConsPerf();
            consumerperf.Initialize(args);
            consumerperf.RegisterShutdownDelegate();
            consumerperf.Run();
            Environment.Exit(0);
        }
    }
}
