/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.PerfTools.Common;
using System.Net;

/**
* <p>
* The ConsPerf application. Implements a consumer, which requests items from a
* provider and process the images and updates it receives. It also includes support
* for posting and generic messages.
* <p>
* The purpose of this application is to measure performance of the ETA transport,
* encoders and decoders in consuming Level I Market Price content directly
* from an OMM provider or through the Refinitiv Real-Time Distribution System.
* </p>
* <em>Summary</em>
* <p>
* The consumer creates two types of threads:
* <ul>
* <li>A main thread, which collects and records statistical information.
* <li>Consumer threads, each of which create a connection to a provider and
*     request market data.
* </ul>
* <p>
* To measure latency, a timestamp is randomly placed in each burst of updates by 
* the provider. The consumer then decodes the timestamp from the update to
* determine the end-to-end latency. ConsPerf also supports measurement of 
* posting latency and generic message latency.
* <p>
* This application also measures memory and CPU usage.
* <p>
* For more detailed information on the performance measurement applications, 
* see the ETA Performance Tools Guide.
* <p>
* <em>Setup Environment</em>
* <p>
* The following configuration files are required:
* <ul>
* <li>RDMFieldDictionary and enumtype.def.
* <li>350k.xml
* <li>MsgData.xml (only required if posting or sending generic msgs)
* </ul>
*/

namespace LSEG.Eta.PerfTools.ConsPerf
{
	public class ConsPerf : IShutdownCallback
	{
		private const int MAX_CONS_THREADS = 8;

		// post user information
		private PostUserInfo m_PostUserInfo = new PostUserInfo();

		// array of consumer threads
		private ConsumerThreadInfo[]? m_ConsumerThreadsInfo;

		// application configuration information
		private ConsPerfConfig m_ConsPerfConfig = new ConsPerfConfig();

		// item information list from XML file
		private XmlItemInfoList? m_XmlItemInfoList;

		// message data information from XML file 
		private XmlMsgData? m_XmlMsgData;

		private ResourceUsageStats m_ResourceUsageStats = new ResourceUsageStats();

		// Total statistics collected from all consumer threads. 
		// This is only used when there are multiple consumer threads. 
		private ConsumerStats m_TotalStats = new ConsumerStats();

		// CPU & Memory Usage samples 
		private ValueStatistics m_CpuUsageStats = new ValueStatistics();
		private ValueStatistics m_MemUsageStats = new ValueStatistics();

		// run-time tracking  
		private double m_CurrentTime;
		private long m_EndTime;
		private int m_CurrentRuntimeSec = 0;
		private	int m_IntervalSeconds = 0;

		// Logs summary information, such as application inputs and final statistics. 
		private StreamWriter? m_SummaryFileWriter = null;

		// stdio writer
		private StreamWriter m_StdOutWriter = new StreamWriter(Console.OpenStandardOutput());

		// indicates whether or not application should be shutdown
		private volatile bool _shutdownApp = false;

		private CountdownEvent _runLoopExited = new CountdownEvent(1);

		/// <summary>
		/// Shutdown ConsPerf
		/// </summary>
		public void Shutdown()
		{
			_shutdownApp = true;
		}

		/// <summary>
		/// Initializes ConsPerf application
		/// </summary>
		/// <param name="args">command line arguments</param>
		private void Initialize(string[] args)
		{
			int threadCount = 1;

			// initialize and print configuration parameters
			m_ConsPerfConfig.Init(args, MAX_CONS_THREADS);
			Console.WriteLine(m_ConsPerfConfig.ToString());

			// parse item list and message data XML files 
			// the application exits if any error occured
			ParseXmlFiles();

			if (m_ConsPerfConfig.PostsPerSec > 0 && m_XmlItemInfoList!.PostMsgItemCount == 0)
			{
				Console.WriteLine("Error: Configured for posting but no posting items found in item file.\n");
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (m_ConsPerfConfig.GenMsgsPerSec > 0 && m_XmlItemInfoList!.GenMsgItemCount == 0)
			{
				Console.WriteLine("Error: Configured for sending generic msgs but no generic msg items found in item file.\n");
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			if (m_ConsPerfConfig.ThreadCount > 0)
			{
				threadCount = m_ConsPerfConfig.ThreadCount;
			}

			m_ConsumerThreadsInfo = new ConsumerThreadInfo[threadCount];

			// Calculate unique index for each thread. Each thread requests a common
			// and unique set of items. Unique index is so each thread has a unique
			// index into the shared item list. 
			int itemListUniqueIndex = m_ConsPerfConfig.CommonItemCount;
			for (int i = 0; i < threadCount; ++i)
			{
				m_ConsumerThreadsInfo[i] = new ConsumerThreadInfo();
				// Figure out how many unique items each consumer should request. 
				m_ConsumerThreadsInfo[i].ItemListCount = m_ConsPerfConfig.ItemRequestCount / threadCount;

				// Distribute remainder.
				if (i < m_ConsPerfConfig.ItemRequestCount % threadCount)
                {
					m_ConsumerThreadsInfo[i].ItemListCount++;
				}

				m_ConsumerThreadsInfo[i].ItemListUniqueIndex = itemListUniqueIndex;
				itemListUniqueIndex += m_ConsumerThreadsInfo[i].ItemListCount - m_ConsPerfConfig.CommonItemCount;
			}

			// create summary file writer
			try
			{
				m_SummaryFileWriter = new StreamWriter(m_ConsPerfConfig.SummaryFilename!);
			}
			catch (Exception)
			{
				Console.WriteLine("Error: Failed to open summary file '{0}'.\n", m_ConsPerfConfig.SummaryFilename);
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}

			//write configuration parameters to summary file
			m_SummaryFileWriter.WriteLine(m_ConsPerfConfig.ToString());
			m_SummaryFileWriter.Flush();

			//set PostUserInfo information.
			if (m_ConsPerfConfig.PostsPerSec > 0)
			{
				try
				{
					m_PostUserInfo.UserId = Thread.CurrentThread.ManagedThreadId;
					m_PostUserInfo.UserAddr = BitConverter.ToUInt32(Dns.GetHostAddresses(Dns.GetHostName())
						.Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)!.FirstOrDefault()?.GetAddressBytes());
				}
				catch (Exception e)
				{
					Console.WriteLine("Failed to get localhost address while setting postUserInfo: {0}\n", e.Message);
					Environment.Exit((int)PerfToolsReturnCode.FAILURE);
				}
			}

			Console.WriteLine("Starting connections ({0} total)...\n", m_ConsPerfConfig.ThreadCount);

			//Spawn consumer threads
			for (int i = 0; i < m_ConsPerfConfig.ThreadCount; ++i)
			{
				m_ConsumerThreadsInfo[i].ThreadId = i + 1;
				new Thread(new ThreadStart(new ConsumerThread(m_ConsumerThreadsInfo[i], m_ConsPerfConfig, m_XmlItemInfoList!, m_XmlMsgData!, m_PostUserInfo, this).Run)).Start();
			}

			// set application end time
			m_EndTime = (long)GetTime.GetNanoseconds() + m_ConsPerfConfig.SteadyStateTime * 1000000000L;

			// Sleep for one more second so some stats can be gathered before first printout. 
			try
			{
				Thread.Sleep(1000);
			}
			catch (Exception e)
			{
				Console.WriteLine("Thread.Sleep(1000) failed: {0}\n", e.Message);
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}
		}

		/// <summary>
		/// Parse item list and message data XML files
		/// </summary>
		private void ParseXmlFiles()
		{
			m_XmlItemInfoList = new XmlItemInfoList(m_ConsPerfConfig.ItemRequestCount);
			m_XmlMsgData = new XmlMsgData();
			if (m_XmlItemInfoList.ParseFile(m_ConsPerfConfig.ItemFilename!) == PerfToolsReturnCode.FAILURE)
			{
				Console.WriteLine("Failed to load item list from file '{0}'.\n", m_ConsPerfConfig.ItemFilename);
				Environment.Exit((int)PerfToolsReturnCode.FAILURE);
			}
			if (m_ConsPerfConfig.PostsPerSec > 0 || m_ConsPerfConfig.GenMsgsPerSec > 0)
			{
				if (m_XmlMsgData.ParseFile(m_ConsPerfConfig.MsgFilename!) == PerfToolsReturnCode.FAILURE)
				{
					Console.WriteLine("Failed to load message data from file '%s'.\n", m_ConsPerfConfig.MsgFilename);
					Environment.Exit((int)PerfToolsReturnCode.FAILURE);
				}
			}
		}

		/// <summary>
		/// Prints Summary statistics
		/// </summary>
		/// <param name="fileWriter"><see cref="StreamWriter"/> instance that provides access to the Summary statistics file</param>
		private void PrintSummaryStatistics(StreamWriter fileWriter)
		{
			long firstUpdateTime;
			long totalUpdateCount = m_TotalStats.StartupUpdateCount.GetTotal() + m_TotalStats.SteadyStateUpdateCount.GetTotal();

			// Find when the first update was received.
			firstUpdateTime = 0;
			for (int i = 0; i < m_ConsPerfConfig.ThreadCount; ++i)
			{
				if (firstUpdateTime == 0 || m_ConsumerThreadsInfo![i].Stats.FirstUpdateTime < firstUpdateTime)
                {
					firstUpdateTime = m_ConsumerThreadsInfo![i].Stats.FirstUpdateTime;
				}
			}

			PrintClientSummaryStats(fileWriter);

			fileWriter.Write("\n--- OVERALL SUMMARY ---\n\n");

			fileWriter.Write("Startup State Statistics:\n");

			double duration = m_TotalStats.ImageRetrievalStartTime > 0 
				? ((m_TotalStats.ImageRetrievalEndTime > 0 ? m_TotalStats.ImageRetrievalEndTime : m_CurrentTime) - m_TotalStats.ImageRetrievalStartTime) / 1000000000.0 
				: 0.0;
			fileWriter.Write("  Sampling duration (sec): {0:0.000}\n", duration);

			if (m_TotalStats.StartupLatencyStats.Count > 0)
			{
				fileWriter.Write("  Latency avg (usec): {0:0.0}\n", m_TotalStats.StartupLatencyStats.Average);
				fileWriter.Write("  Latency std dev (usec): {0:0.0}\n", Math.Sqrt(m_TotalStats.StartupLatencyStats.Variance));
				fileWriter.Write("  Latency max (usec): {0:0.0}\n", m_TotalStats.StartupLatencyStats.MaxValue);
				fileWriter.Write("  Latency min (usec): {0:0.0}\n", m_TotalStats.StartupLatencyStats.MinValue);
			}
			else
            {
				fileWriter.Write("  No latency information received during startup time.\n\n");
			}

			double updRate = m_TotalStats.StartupUpdateCount.GetTotal() / (((m_TotalStats.ImageRetrievalEndTime > 0 ? m_TotalStats.ImageRetrievalEndTime : m_CurrentTime) - firstUpdateTime) / 1000000000.0);
			fileWriter.Write("  Avg update rate: {0:0.0}\n\n", updRate);

			fileWriter.Write("Steady State Statistics:\n");

			if (m_TotalStats.ImageRetrievalEndTime > 0)
			{

				fileWriter.Write("  Sampling duration (sec): {0:0.000}\n", (m_CurrentTime - m_TotalStats.ImageRetrievalEndTime) / 1000000000.0);

				if (m_TotalStats.SteadyStateLatencyStats.Count > 0)
				{
					fileWriter.Write("  Latency avg (usec): {0:0.0}\n", m_TotalStats.SteadyStateLatencyStats.Average);
					fileWriter.Write("  Latency std dev (usec): {0:0.0}\n", Math.Sqrt(m_TotalStats.SteadyStateLatencyStats.Variance));
					fileWriter.Write("  Latency max (usec): {0:0.0}\n", m_TotalStats.SteadyStateLatencyStats.MaxValue);
					fileWriter.Write("  Latency min (usec): {0:0.0}\n", m_TotalStats.SteadyStateLatencyStats.MinValue);
				}
				else
                {
					fileWriter.Write("  No latency information was received during steady-state time.\n");
				}
					

				if (m_ConsPerfConfig.LatencyPostsPerSec > 0)
				{
					if (m_TotalStats.PostLatencyStats.Count > 0)
					{
						fileWriter.Write("  Posting latency avg (usec): {0:0.0}\n", m_TotalStats.PostLatencyStats.Average);
						fileWriter.Write("  Posting latency std dev (usec): {0:0.0}\n", Math.Sqrt(m_TotalStats.PostLatencyStats.Variance));
						fileWriter.Write("  Posting latency max (usec): {0:0.0}\n", m_TotalStats.PostLatencyStats.MaxValue);
						fileWriter.Write("  Posting latency min (usec): {0:0.0}\n", m_TotalStats.PostLatencyStats.MinValue);
					}
					else
                    {
						fileWriter.Write("  No posting latency information was received during steady-state time.\n");
					}
				}

				fileWriter.Write("  Avg update rate: {0:0.0}\n", 
					m_TotalStats.SteadyStateUpdateCount.GetTotal() / ((m_CurrentTime - m_TotalStats.ImageRetrievalEndTime) / 1000000000.0));
				fileWriter.Write("\n");
			}
			else
            {
				fileWriter.Write("  Steady state was not reached during this test.\n\n");
			}
				

			fileWriter.Write("Overall Statistics: \n");

			fileWriter.Write("  Sampling duration (sec): {0:0.000}\n",
					m_TotalStats.ImageRetrievalStartTime > 0 ? (m_CurrentTime - m_TotalStats.ImageRetrievalStartTime) / 1000000000.0 : 0.0);

			if (m_TotalStats.OverallLatencyStats.Count > 0)
			{
				fileWriter.Write("  Latency avg (usec): {0:0.0}\n", m_TotalStats.OverallLatencyStats.Average);
				fileWriter.Write("  Latency std dev (usec): {0:0.0}\n", Math.Sqrt(m_TotalStats.OverallLatencyStats.Variance));
				fileWriter.Write("  Latency max (usec): {0:0.0}\n", m_TotalStats.OverallLatencyStats.MaxValue);
				fileWriter.Write("  Latency min (usec): {0:0.0}\n", m_TotalStats.OverallLatencyStats.MinValue);
			}
			else
            {
				fileWriter.Write("  No latency information was received.\n");
			}

			if (m_TotalStats.GenMsgLatencyStats.Count > 0)
			{
				fileWriter.Write("  GenMsg latency avg (usec): {0:0.0}\n", m_TotalStats.GenMsgLatencyStats.Average);
				fileWriter.Write("  GenMsg latency std dev (usec): {0:0.0}\n", Math.Sqrt(m_TotalStats.GenMsgLatencyStats.Variance));
				fileWriter.Write("  GenMsg latency max (usec): {0:0.0}\n", m_TotalStats.GenMsgLatencyStats.MaxValue);
				fileWriter.Write("  GenMsg latency min (usec): {0:0.0}\n", m_TotalStats.GenMsgLatencyStats.MinValue);
			}
			else
            {
				fileWriter.Write("  No GenMsg latency information was received.\n");
			}

			if (m_CpuUsageStats.Count > 0)
			{
				fileWriter.Write("  CPU/Memory Samples: {0}\n", m_CpuUsageStats.Count);
				fileWriter.Write("  CPU Usage max (%%): {0:0.00}\n", m_CpuUsageStats.MaxValue);
				fileWriter.Write("  CPU Usage min (%%): {0:0.00}\n", m_CpuUsageStats.MinValue);
				fileWriter.Write("  CPU Usage avg (%%): {0:0.00}\n", m_CpuUsageStats.Average);
				fileWriter.Write("  Memory Usage max (MB): {0:0.00}\n", m_MemUsageStats.MaxValue);
				fileWriter.Write("  Memory Usage min (MB): {0:0.00}\n", m_MemUsageStats.MinValue);
				fileWriter.Write("  Memory Usage avg (MB): {0:0.00}\n", m_MemUsageStats.Average);
			}

			fileWriter.Write("\nTest Statistics:\n");

			fileWriter.Write("  Requests sent: {0}\n", m_TotalStats.RequestCount.GetTotal());
			fileWriter.Write("  Refreshes received: {0}\n", m_TotalStats.RefreshCount.GetTotal());
			fileWriter.Write("  Updates received: {0}\n", totalUpdateCount);

			if (m_ConsPerfConfig.PostsPerSec > 0)
			{
				fileWriter.Write("  Posts sent: {0}\n", m_TotalStats.PostSentCount.GetTotal());
			}

			if (m_ConsPerfConfig.GenMsgsPerSec > 0)
            {
				fileWriter.Write("  GenMsgs sent: {0}\n", m_TotalStats.GenMsgSentCount.GetTotal());
			}				
			if (m_TotalStats.GenMsgRecvCount.GetTotal() > 0)
            {
				fileWriter.Write("  GenMsgs received: {0}\n", m_TotalStats.GenMsgRecvCount.GetTotal());
			}				
			if (m_ConsPerfConfig.LatencyGenMsgsPerSec > 0)
            {
				fileWriter.Write("  GenMsg latencies sent: {0}\n", m_TotalStats.LatencyGenMsgSentCount.GetTotal());
			}			
			if (m_TotalStats.GenMsgLatencyStats.Count > 0)
            {
				fileWriter.Write("  GenMsg latencies received: {0}\n", m_TotalStats.GenMsgLatencyStats.Count);
			}				

			if (m_TotalStats.ImageRetrievalEndTime > 0)
			{
				long totalRefreshRetrievalTime = m_TotalStats.ImageRetrievalEndTime - m_TotalStats.ImageRetrievalStartTime;

				fileWriter.Write("  Image retrieval time (sec): {0:0.000}\n", totalRefreshRetrievalTime / 1000000000.0);
				fileWriter.Write("  Avg image Rate: {0:0.0}\n", m_ConsPerfConfig.ItemRequestCount / (totalRefreshRetrievalTime / 1000000000.0));
			}

			fileWriter.Write("  Avg update rate: {0:0.0}", totalUpdateCount / ((m_CurrentTime - firstUpdateTime) / 1000000000.0));

			if (m_TotalStats.PostSentCount.GetTotal() > 0)
			{
				fileWriter.Write("  Avg posting rate: {0:0.0}\n", m_TotalStats.PostSentCount.GetTotal() / ((m_CurrentTime - m_TotalStats.ImageRetrievalEndTime) / 1000000000.0));
			}

			if (m_ConsPerfConfig.GenMsgsPerSec > 0)
			{
				fileWriter.Write("  Avg GenMsg send rate: {0:0.0}\n", m_TotalStats.GenMsgSentCount.GetTotal() / ((m_CurrentTime - m_TotalStats.ImageRetrievalEndTime) / 1000000000.0));
			}
			if (m_TotalStats.GenMsgRecvCount.GetTotal() > 0)
			{
				fileWriter.Write("  Avg GenMsg receive rate: {0:0.0}\n", m_TotalStats.GenMsgRecvCount.GetTotal() / ((m_CurrentTime - m_TotalStats.ImageRetrievalEndTime) / 1000000000.0));
			}
			if (m_ConsPerfConfig.LatencyGenMsgsPerSec > 0)
			{
				fileWriter.Write("  Avg GenMsg latency send rate: {0:0.0}\n", m_TotalStats.LatencyGenMsgSentCount.GetTotal() / ((m_CurrentTime - m_TotalStats.ImageRetrievalEndTime) / 1000000000.0));
			}
			if (m_TotalStats.GenMsgLatencyStats.Count > 0)
			{
				fileWriter.Write("  Avg GenMsg latency receive rate: {0:0.0}\n", m_TotalStats.GenMsgLatencyStats.Count / ((m_CurrentTime - m_TotalStats.ImageRetrievalEndTime) / 1000000000.0));
			}

			fileWriter.Write("\n");
		}

		/// <summary>
		/// Print client summary statistics
		/// </summary>
		/// <param name="fileWriter">the file stream for statistics output</param>
		private void PrintClientSummaryStats(StreamWriter fileWriter)
		{
			for (int i = 0; i < m_ConsPerfConfig.ThreadCount; ++i)
			{
				long imageRetrievalTime = (m_ConsumerThreadsInfo![i].Stats.ImageRetrievalEndTime > 0) 
					? m_ConsumerThreadsInfo[i].Stats.ImageRetrievalEndTime - m_ConsumerThreadsInfo[i].Stats.ImageRetrievalStartTime
					: 0;
				double endTime = m_ConsumerThreadsInfo[i].Stats.ImageRetrievalEndTime > 0 ? m_ConsumerThreadsInfo[i].Stats.ImageRetrievalEndTime : m_CurrentTime;
				bool imageRetrievalStarted = m_ConsumerThreadsInfo[i].Stats.ImageRetrievalStartTime > 0;
				double elapsedTime = (m_CurrentTime - m_ConsumerThreadsInfo[i].Stats.ImageRetrievalEndTime) / 1000000000.0;

				// If there are multiple connections, print individual summaries.
				if (m_ConsPerfConfig.ThreadCount > 1)
				{
					long totalClientUpdateCount = m_ConsumerThreadsInfo[i].Stats.StartupUpdateCount.GetTotal() + m_ConsumerThreadsInfo[i].Stats.SteadyStateUpdateCount.GetTotal();

					fileWriter.Write("\n--- CLIENT {0} SUMMARY ---\n\n", i + 1);
					fileWriter.Write("Startup State Statistics:\n");
					fileWriter.Write("  Sampling duration (sec): {0:0.000}\n", imageRetrievalStarted ? (endTime - m_ConsumerThreadsInfo[i].Stats.ImageRetrievalStartTime) / 1000000000.0 : 0.0);

					if (m_ConsumerThreadsInfo[i].Stats.StartupLatencyStats.Count > 0)
					{
						fileWriter.Write("  Latency avg (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.StartupLatencyStats.Average);
						fileWriter.Write("  Latency std dev (usec): {0:0.0}\n", Math.Sqrt(m_ConsumerThreadsInfo[i].Stats.StartupLatencyStats.Variance));
						fileWriter.Write("  Latency max (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.StartupLatencyStats.MaxValue);
						fileWriter.Write("  Latency min (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.StartupLatencyStats.MinValue);
					}
					else
                    {
						fileWriter.Write("  No latency information received during startup time.\n\n");
					}

					fileWriter.Write("  Avg update rate: {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.StartupUpdateCount.GetTotal() / ((endTime - m_ConsumerThreadsInfo[i].Stats.FirstUpdateTime) / 1000000000.0));

					fileWriter.Write("\nSteady State Statistics:\n");

					if (m_ConsumerThreadsInfo[i].Stats.ImageRetrievalEndTime > 0)
					{
						fileWriter.Write("  Sampling duration (sec): {0:0.000}\n", elapsedTime);
						if (m_ConsumerThreadsInfo[i].Stats.SteadyStateLatencyStats.Count > 0)
						{
							fileWriter.Write("  Latency avg (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.SteadyStateLatencyStats.Average);
							fileWriter.Write("  Latency std dev (usec): {0:0.0}\n", Math.Sqrt(m_ConsumerThreadsInfo[i].Stats.SteadyStateLatencyStats.Variance));
							fileWriter.Write("  Latency max (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.SteadyStateLatencyStats.MaxValue);
							fileWriter.Write("  Latency min (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.SteadyStateLatencyStats.MinValue);
						}
						else
                        {
							fileWriter.Write("  No latency information was received during steady-state time.\n");
						}

						if (m_ConsPerfConfig.LatencyPostsPerSec > 0)
						{
							if (m_ConsumerThreadsInfo[i].Stats.PostLatencyStats.Count > 0)
							{
								fileWriter.Write("  Posting latency avg (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.PostLatencyStats.Average);
								fileWriter.Write("  Posting latency std dev (usec): {0:0.0}\n", Math.Sqrt(m_ConsumerThreadsInfo[i].Stats.PostLatencyStats.Variance));
								fileWriter.Write("  Posting latency max (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.PostLatencyStats.MaxValue);
								fileWriter.Write("  Posting latency min (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.PostLatencyStats.MinValue);
							}
							else
                            {
								fileWriter.Write("  No posting latency information was received during steady-state time.\n");
							}
						}

						fileWriter.Write("  Avg update rate: {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.SteadyStateUpdateCount.GetTotal() / elapsedTime);
					}
					else
                    {
						fileWriter.Write("  Steady state was not reached during this test.\n\n");
					}

					fileWriter.Write("\nOverall Statistics: \n");
					fileWriter.Write("  Sampling duration (sec): {0:0.000}\n", imageRetrievalStarted ? (m_CurrentTime - m_ConsumerThreadsInfo[i].Stats.ImageRetrievalStartTime) / 1000000000.0 : 0.0);

					if (m_ConsumerThreadsInfo[i].Stats.OverallLatencyStats.Count > 0)
					{
						fileWriter.Write("  Latency avg (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.OverallLatencyStats.Average);
						fileWriter.Write("  Latency std dev (usec): {0:0.0}\n", Math.Sqrt(m_ConsumerThreadsInfo[i].Stats.OverallLatencyStats.Variance));
						fileWriter.Write("  Latency max (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.OverallLatencyStats.MaxValue);
						fileWriter.Write("  Latency min (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.OverallLatencyStats.MinValue);
					}
					else
                    {
						fileWriter.Write("  No latency information was received.\n");
					}

					if (m_ConsPerfConfig.LatencyGenMsgsPerSec > 0)
					{
						if (m_ConsumerThreadsInfo[i].Stats.GenMsgLatencyStats.Count > 0)
						{
							fileWriter.Write("  GenMsg latency avg (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.GenMsgLatencyStats.Average);
							fileWriter.Write("  GenMsg latency std dev (usec): {0:0.0}\n", Math.Sqrt(m_ConsumerThreadsInfo[i].Stats.GenMsgLatencyStats.Variance));
							fileWriter.Write("  GenMsg latency max (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.GenMsgLatencyStats.MaxValue);
							fileWriter.Write("  GenMsg latency min (usec): {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.GenMsgLatencyStats.MinValue);
						}
						else
                        {
							fileWriter.Write("  No GenMsg latency information was received.\n");
						}
					}

					fileWriter.Write("\nTest Statistics:\n");

					fileWriter.Write("  Requests sent: {0}\n", m_ConsumerThreadsInfo[i].Stats.RequestCount.GetTotal());
					fileWriter.Write("  Refreshes received: {0}\n", m_ConsumerThreadsInfo[i].Stats.RefreshCount.GetTotal());
					fileWriter.Write("  Updates received: {0}\n", totalClientUpdateCount);

					if (m_ConsPerfConfig.PostsPerSec > 0)
					{
						fileWriter.Write("  Posts sent: {0}\n", m_ConsumerThreadsInfo[i].Stats.PostSentCount.GetTotal());
					}

					if (m_ConsPerfConfig.GenMsgsPerSec > 0)
						fileWriter.Write("  GenMsgs sent: {0}\n", m_ConsumerThreadsInfo[i].Stats.GenMsgSentCount.GetTotal());
					if (m_ConsumerThreadsInfo[i].Stats.GenMsgRecvCount.GetTotal() > 0)
						fileWriter.Write("  GenMsgs received: {0}\n", m_ConsumerThreadsInfo[i].Stats.GenMsgRecvCount.GetTotal());
					if (m_ConsPerfConfig.LatencyGenMsgsPerSec > 0)
						fileWriter.Write("  GenMsg latencies sent: {0}\n", m_ConsumerThreadsInfo[i].Stats.LatencyGenMsgSentCount.GetTotal());
					if (m_ConsumerThreadsInfo[i].Stats.GenMsgLatencyStats.Count > 0)
						fileWriter.Write("  GenMsg latencies received: {0}\n", m_ConsumerThreadsInfo[i].Stats.GenMsgLatencyStats.Count);

					if (imageRetrievalTime > 0)
					{
						fileWriter.Write("  Image retrieval time(sec): {0:0.000}\n", imageRetrievalTime / 1000000000.0);
						fileWriter.Write("  Avg image Rate: {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.RefreshCount.GetTotal() / (imageRetrievalTime / 1000000000.0));
					}

					fileWriter.Write("  Avg update rate: {0:0.0}\n", totalClientUpdateCount / ((m_CurrentTime - m_ConsumerThreadsInfo[i].Stats.FirstUpdateTime) / 1000000000.0));

					if (m_ConsPerfConfig.PostsPerSec > 0)
					{
						fileWriter.Write("  Avg posting rate: {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.PostSentCount.GetTotal() / elapsedTime);
					}
					if (m_ConsPerfConfig.GenMsgsPerSec > 0)
					{
						fileWriter.Write("  Avg GenMsg send rate: {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.GenMsgSentCount.GetTotal() / elapsedTime);
					}
					if (m_ConsumerThreadsInfo[i].Stats.GenMsgRecvCount.GetTotal() > 0)
					{
						fileWriter.Write("  Avg GenMsg receive rate: {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.GenMsgRecvCount.GetTotal() / elapsedTime);
					}
					if (m_ConsPerfConfig.LatencyGenMsgsPerSec > 0)
					{
						fileWriter.Write("  Avg GenMsg latency send rate: {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.LatencyGenMsgSentCount.GetTotal() / elapsedTime);
					}
					if (m_ConsumerThreadsInfo[i].Stats.GenMsgLatencyStats.Count > 0)
					{
						fileWriter.Write("  Avg GenMsg latency receive rate: {0:0.0}\n", m_ConsumerThreadsInfo[i].Stats.GenMsgLatencyStats.Count / elapsedTime);
					}
				}
			}
		}

		/// <summary>
		/// Collect statistics
		/// </summary>
		/// <param name="writeStats">determines whether to write the statistics into the file</param>
		/// <param name="displayStats">determines whether to display the statistics </param>
		/// <param name="currentRuntimeSec">current runtime</param>
		/// <param name="timePassedSec"></param>
		private void CollectStats(bool writeStats, bool displayStats, int currentRuntimeSec, long timePassedSec)
		{
			bool allRefreshesRetrieved = true;

			m_ResourceUsageStats.Refresh();
			double processCpuLoad = m_ResourceUsageStats.CurrentProcessCpuLoad();
			double memoryUsage = m_ResourceUsageStats.CurrentMemoryUsage();

			m_CpuUsageStats.Update(processCpuLoad);
			m_MemUsageStats.Update(memoryUsage);

			for (int i = 0; i < m_ConsPerfConfig.ThreadCount; i++)
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

				// Gather latency records from each thread and update statistics.
				CollectUpdateStats(m_ConsumerThreadsInfo![i]);

				// Gather latency records for posts.
				CollectPostStats(m_ConsumerThreadsInfo[i]);

				// Gather latency records for generic msgs.
				CollectGenMsgStats(m_ConsumerThreadsInfo[i]);

				if (m_ConsumerThreadsInfo[i].LatencyLogFile != null)
                {
					m_ConsumerThreadsInfo[i].LatencyLogFileWriter!.Flush();
				}

				// Collect counts.
				startupUpdateCount = m_ConsumerThreadsInfo[i].Stats.StartupUpdateCount.GetChange();
				steadyStateUpdateCount = m_ConsumerThreadsInfo[i].Stats.SteadyStateUpdateCount.GetChange();
				statusCount = m_ConsumerThreadsInfo[i].Stats.StatusCount.GetChange();
				requestCount = m_ConsumerThreadsInfo[i].Stats.RequestCount.GetChange();
				refreshCount = m_ConsumerThreadsInfo[i].Stats.RefreshCount.GetChange();
				postOutOfBuffersCount = m_ConsumerThreadsInfo[i].Stats.GenMsgOutOfBuffersCount.GetChange();
				postSentCount = m_ConsumerThreadsInfo[i].Stats.PostSentCount.GetChange();
				genMsgSentCount = m_ConsumerThreadsInfo[i].Stats.GenMsgSentCount.GetChange();
				genMsgRecvCount = m_ConsumerThreadsInfo[i].Stats.GenMsgRecvCount.GetChange();
				latencyGenMsgSentCount = m_ConsumerThreadsInfo[i].Stats.LatencyGenMsgSentCount.GetChange();
				latencyGenMsgRecvCount = m_ConsumerThreadsInfo[i].Stats.IntervalGenMsgLatencyStats.Count;
				genMsgOutOfBuffersCount = m_ConsumerThreadsInfo[i].Stats.PostOutOfBuffersCount.GetChange();

				if (m_ConsPerfConfig.ThreadCount > 1)
				{
					m_TotalStats.StartupUpdateCount.Add(startupUpdateCount);
					m_TotalStats.SteadyStateUpdateCount.Add(steadyStateUpdateCount);
					m_TotalStats.StatusCount.Add(statusCount);
					m_TotalStats.RequestCount.Add(requestCount);
					m_TotalStats.RefreshCount.Add(refreshCount);
					m_TotalStats.PostOutOfBuffersCount.Add(postOutOfBuffersCount);
					m_TotalStats.PostSentCount.Add(postSentCount);
					m_TotalStats.GenMsgSentCount.Add(genMsgSentCount);
					m_TotalStats.GenMsgRecvCount.Add(genMsgRecvCount);
					m_TotalStats.LatencyGenMsgSentCount.Add(latencyGenMsgSentCount);
					m_TotalStats.GenMsgOutOfBuffersCount.Add(genMsgOutOfBuffersCount);
				}

				if (writeStats)
				{
					// Log statistics to file.
					PrintCurrentTimeUTC(m_ConsumerThreadsInfo[i].StatsFileWriter!);
					m_ConsumerThreadsInfo[i].StatsFileWriter!.Write(
							", {0}, {1:0.0}, {2:0.0}, {3:0.0}, {4:0.0}, {5}, {6}, {7}, {8:0.0}, {9:0.0}, {10:0.0}, {11:0.0}, {12}, {13}, {14}, {15}, {16:0.0}, {17:0.0}, {18:0.0}, {19:0.0}, {20:0.00}, {21:0.00}\n",
							m_ConsumerThreadsInfo[i].Stats.IntervalLatencyStats.Count,
							m_ConsumerThreadsInfo[i].Stats.IntervalLatencyStats.Average,
							Math.Sqrt(m_ConsumerThreadsInfo[i].Stats.IntervalLatencyStats.Variance),
							m_ConsumerThreadsInfo[i].Stats.IntervalLatencyStats.Count > 0 ? m_ConsumerThreadsInfo[i].Stats.IntervalLatencyStats.MaxValue : 0.0,
							m_ConsumerThreadsInfo[i].Stats.IntervalLatencyStats.Count > 0 ? m_ConsumerThreadsInfo[i].Stats.IntervalLatencyStats.MinValue : 0.0,
							refreshCount,
							(startupUpdateCount + steadyStateUpdateCount) / timePassedSec,
							m_ConsumerThreadsInfo[i].Stats.IntervalPostLatencyStats.Count,
							m_ConsumerThreadsInfo[i].Stats.IntervalPostLatencyStats.Average,
							Math.Sqrt(m_ConsumerThreadsInfo[i].Stats.IntervalPostLatencyStats.Variance),
							m_ConsumerThreadsInfo[i].Stats.IntervalPostLatencyStats.Count > 0 ? m_ConsumerThreadsInfo[i].Stats.IntervalPostLatencyStats.MaxValue : 0.0,
							m_ConsumerThreadsInfo[i].Stats.IntervalPostLatencyStats.Count > 0 ? m_ConsumerThreadsInfo[i].Stats.IntervalPostLatencyStats.MinValue : 0.0,
							genMsgSentCount,
							genMsgRecvCount,
							latencyGenMsgSentCount,
							latencyGenMsgRecvCount,
							m_ConsumerThreadsInfo[i].Stats.IntervalGenMsgLatencyStats.Average,
							Math.Sqrt(m_ConsumerThreadsInfo[i].Stats.IntervalGenMsgLatencyStats.Variance),
							m_ConsumerThreadsInfo[i].Stats.IntervalGenMsgLatencyStats.Count > 0 ? m_ConsumerThreadsInfo[i].Stats.IntervalGenMsgLatencyStats.MaxValue : 0.0,
							m_ConsumerThreadsInfo[i].Stats.IntervalGenMsgLatencyStats.Count > 0 ? m_ConsumerThreadsInfo[i].Stats.IntervalGenMsgLatencyStats.MinValue : 0.0,
							processCpuLoad,
							memoryUsage
							);
				}

				if (displayStats)
				{
					if (m_ConsPerfConfig.ThreadCount == 1)
						Console.Write("{0}: ", currentRuntimeSec);
					else
						Console.Write("{0}: Client {1}:\n  ", currentRuntimeSec, i + 1);

					Console.Write("Images: {0}, Posts: {1}, UpdRate: {2}, CPU: {3:0.00}, Mem: {4:000.00}MB\n",
							refreshCount,
							postSentCount,
							(startupUpdateCount + steadyStateUpdateCount) / timePassedSec,
							processCpuLoad,
							memoryUsage);

					if (m_ConsumerThreadsInfo[i].Stats.IntervalLatencyStats.Count > 0)
					{
						m_ConsumerThreadsInfo[i].Stats.IntervalLatencyStats.Print("  Latency(usec)", "Msgs", false);
						m_ConsumerThreadsInfo[i].Stats.IntervalLatencyStats.Clear();
					}

					if (postOutOfBuffersCount > 0)
                    {
						Console.Write("  - {0} posts not sent due to lack of output buffers.\n", postOutOfBuffersCount);
					}

					if (m_ConsumerThreadsInfo[i].Stats.IntervalPostLatencyStats.Count > 0)
					{
						m_ConsumerThreadsInfo[i].Stats.IntervalPostLatencyStats.Print("  PostLat(usec)", "Msgs", false);
						m_ConsumerThreadsInfo[i].Stats.IntervalPostLatencyStats.Clear();
					}

					if (genMsgSentCount > 0 || genMsgRecvCount > 0)
                    {
						Console.Write("  GenMsgs: sent {0}, received {1}, latencies sent {2}, latencies received {3}\n", genMsgSentCount, genMsgRecvCount, latencyGenMsgSentCount, latencyGenMsgRecvCount);
					}

					if (genMsgOutOfBuffersCount > 0)
                    {
						Console.Write("  - {0} GenMsgs not sent due to lack of output buffers.\n", genMsgOutOfBuffersCount);
					}						

					if (m_ConsumerThreadsInfo[i].Stats.IntervalGenMsgLatencyStats.Count > 0)
					{
						m_ConsumerThreadsInfo[i].Stats.IntervalGenMsgLatencyStats.Print("  GenMsgLat(usec)", "Msgs", false);
						m_ConsumerThreadsInfo[i].Stats.IntervalGenMsgLatencyStats.Clear();
					}

					if (m_ConsumerThreadsInfo[i].Stats.TunnelStreamBufUsageStats.Count > 0)
					{
						m_ConsumerThreadsInfo[i].Stats.TunnelStreamBufUsageStats.Print("  TunnelStreamBufferUsed", "Samples", false);
						m_ConsumerThreadsInfo[i].Stats.TunnelStreamBufUsageStats.Clear();
					}

					if (statusCount > 0)
                    {
						Console.WriteLine("  - Received {0} status messages.\n", statusCount);
					}
				}

				// Get Image Retrieval time for this client.
				if (GetClientImageRetrievalTime(m_ConsumerThreadsInfo[i], displayStats) == false)
				{
					allRefreshesRetrieved = false;
				}
			}

			if (!m_TotalStats.ImageTimeRecorded && allRefreshesRetrieved && m_ConsPerfConfig.ThreadCount > 0)
			{
				m_EndTime = m_TotalStats.ImageRetrievalEndTime + m_ConsPerfConfig.SteadyStateTime * 1000000000L;
				m_TotalStats.ImageTimeRecorded = true;

				if (m_ConsPerfConfig.ThreadCount > 1)
				{
					if (displayStats)
					{
						// Print overall image retrieval stats.
						long totalRefreshRetrievalTime = m_TotalStats.ImageRetrievalEndTime - m_TotalStats.ImageRetrievalStartTime;

						Console.Write("\nOverall image retrieval time for {0} images: {1:0.000}s ({2} Images/s).\n\n",
								m_ConsPerfConfig.ItemRequestCount,
								totalRefreshRetrievalTime / 1000000000.0,
								m_ConsPerfConfig.ItemRequestCount /
								(totalRefreshRetrievalTime / 1000000000.0)
							  );
					}
				}
				else
				{
					m_TotalStats.ImageRetrievalStartTime = m_ConsumerThreadsInfo![0].Stats.ImageRetrievalStartTime;
					m_TotalStats.ImageRetrievalEndTime = m_ConsumerThreadsInfo[0].Stats.ImageRetrievalEndTime;
					m_TotalStats.SteadyStateLatencyTime = m_TotalStats.ImageRetrievalEndTime + m_ConsPerfConfig.DelaySteadyStateCalc * 1000000L;
				}
			}
		}

		/// <summary>
		/// Collect update statistics
		/// </summary>
		/// <param name="consumerThread">Current thread</param>
		private void CollectUpdateStats(ConsumerThreadInfo consumerThread)
		{
			TimeRecordQueue latencyRecords = consumerThread.LatencyRecords;

			while (!latencyRecords.Records.IsEmpty)
			{
				TimeRecord? record;
				if (latencyRecords.Records.TryDequeue(out record))
				{
					double latency = (double)((double)record.EndTime - (double)record.StartTime) / (double)record.Ticks;
					double recordEndTimeNsec = (double)record.EndTime / (double)record.Ticks * 1000.0;

					// Make sure this latency is counted towards startup or steady-state as appropriate.
					bool latencyIsSteadyStateForClient = consumerThread.Stats.ImageRetrievalEndTime != 0 && recordEndTimeNsec > consumerThread.Stats.ImageRetrievalEndTime;

					consumerThread.Stats.IntervalLatencyStats.Update(latency);
					consumerThread.Stats.OverallLatencyStats.Update(latency);
					if (latencyIsSteadyStateForClient)
					{
						if (recordEndTimeNsec > consumerThread.Stats.SteadyStateLatencyTime)
						{
							consumerThread.Stats.SteadyStateLatencyStats.Update(latency);
						}
					}
					else
					{
						consumerThread.Stats.StartupLatencyStats.Update(latency);
					}

					if (m_ConsPerfConfig.ThreadCount > 1)
					{
						// Make sure this latency is counted towards startup or steady-state as appropriate.
						bool latencyIsSteadyStateOverall = m_TotalStats.ImageRetrievalEndTime != 0 && recordEndTimeNsec > m_TotalStats.ImageRetrievalEndTime;

						if (latencyIsSteadyStateOverall)
						{
							if (recordEndTimeNsec > m_TotalStats.SteadyStateLatencyTime)
							{
								m_TotalStats.SteadyStateLatencyStats.Update(latency);
							}
						}
						else
						{
							m_TotalStats.StartupLatencyStats.Update(latency);
						}
						m_TotalStats.OverallLatencyStats.Update(latency);
					}

					if (consumerThread.LatencyLogFile != null)
					{
						consumerThread.LatencyLogFileWriter!.Write("Upd, {0}, {1}, {2}\n", record.StartTime, record.EndTime, record.EndTime - record.StartTime);
					}

					latencyRecords.Pools.Enqueue(record);
				}
			}
		}

		/// <summary>
		/// Collects post statistics
		/// </summary>
		/// <param name="consumerThread">The current thread</param>
		private void CollectPostStats(ConsumerThreadInfo consumerThread)
		{
			TimeRecordQueue latencyRecords = consumerThread.PostLatencyRecords;
			while (!latencyRecords.Records.IsEmpty)
			{
				TimeRecord? record;
				if (latencyRecords.Records.TryDequeue(out record))
				{
					double latency = ((double)record.EndTime - (double)record.StartTime / (double)record.Ticks);

					consumerThread.Stats.IntervalPostLatencyStats.Update(latency);
					consumerThread.Stats.PostLatencyStats.Update(latency);

					if (m_ConsPerfConfig.ThreadCount > 1)
					{
						m_TotalStats.PostLatencyStats.Update(latency);
					}
					if (consumerThread.LatencyLogFile != null)
					{
						consumerThread.LatencyLogFileWriter!.Write("Pst, {0}, {1}, {2}\n", record.StartTime, record.EndTime, record.EndTime - record.StartTime);
					}

					latencyRecords.Pools.Enqueue(record);
				}
			}
		}

		/// <summary>
		/// Collect generic message statistics
		/// </summary>
		/// <param name="consumerThread">Current thread</param>
		private void CollectGenMsgStats(ConsumerThreadInfo consumerThread)
		{
			TimeRecordQueue latencyRecords = consumerThread.GenMsgLatencyRecords;
			while (!latencyRecords.Records.IsEmpty)
			{
				TimeRecord? record;
				if (latencyRecords.Records.TryDequeue(out record))
				{
					double latency = (double)(record.EndTime - record.StartTime) / (double)record.Ticks;

					consumerThread.Stats.IntervalGenMsgLatencyStats.Update(latency);
					consumerThread.Stats.GenMsgLatencyStats.Update(latency);

					if (m_ConsPerfConfig.ThreadCount > 1)
					{
						m_TotalStats.GenMsgLatencyStats.Update(latency);
					}

					if (consumerThread.LatencyLogFile != null)
						consumerThread.LatencyLogFileWriter!.Write("Gen, {0}, {1}, {2}\n", record.StartTime, record.EndTime, record.EndTime - record.StartTime);

					latencyRecords.Pools.Enqueue(record);
				}
			}
		}

		/// <summary>
		/// Get client image retrieval time
		/// </summary>
		/// <param name="consumerThread">current consumer thread</param>
		/// <param name="displayStats">determines whether the statistics will be displayed</param>
		/// <returns>true if all refreshes were retrieved, false otherwise</returns>
		private bool GetClientImageRetrievalTime(ConsumerThreadInfo consumerThread, bool displayStats)
		{
			bool allRefreshesRetrieved = true;

			if (!m_TotalStats.ImageTimeRecorded)
			{
				if (consumerThread.Stats.ImageRetrievalEndTime > 0)
				{
					long imageRetrievalStartTime = consumerThread.Stats.ImageRetrievalStartTime;
					long imageRetrievalEndTime = consumerThread.Stats.ImageRetrievalEndTime;

					// To get the total time it took to retrieve all images, find the earliest start time
					// and latest end time across all connections.
					if (m_TotalStats.ImageRetrievalStartTime == 0 || imageRetrievalStartTime < m_TotalStats.ImageRetrievalStartTime)
                    {
						m_TotalStats.ImageRetrievalStartTime = imageRetrievalStartTime;
					}
					if (m_TotalStats.ImageRetrievalEndTime == 0 || imageRetrievalEndTime > m_TotalStats.ImageRetrievalEndTime)
                    {
						m_TotalStats.ImageRetrievalEndTime = imageRetrievalEndTime;
					}
				}
				else if (consumerThread.ItemListCount > 0)		// Ignore connections that don't request anything.
				{
					allRefreshesRetrieved = false;				// Not all connections have received their images yet.
					m_TotalStats.ImageRetrievalStartTime = 0;
					m_TotalStats.ImageRetrievalEndTime = 0;
				}
			}

			if (!consumerThread.Stats.ImageTimeRecorded && consumerThread.Stats.ImageRetrievalEndTime > 0)
			{
				consumerThread.Stats.ImageTimeRecorded = true;

				if (displayStats)
				{
					long imageRetrievalTime = consumerThread.Stats.ImageRetrievalEndTime - consumerThread.Stats.ImageRetrievalStartTime;

					Console.Write("  - Image retrieval time for {0} images: {1:0.000}s ({2:0.0} images/s)\n",
							consumerThread.ItemListCount,
							imageRetrievalTime / 1000000000.0,
							consumerThread.ItemListCount / (imageRetrievalTime / 1000000000.0));
				}
			}

			return allRefreshesRetrieved;
		}

		/// <summary>
		/// Prints the current UTC time
		/// </summary>
		/// <param name="fileWriter">the stream writer for statistics output</param>
		private void PrintCurrentTimeUTC(StreamWriter fileWriter)
		{
			var nowTime = System.DateTime.UtcNow;
			fileWriter.Write(nowTime.ToString());
		}

		/// <summary>
		/// Stop all consumer threads
		/// </summary>
		private void StopConsumerThreads()
		{
			for (int i = 0; i < m_ConsPerfConfig.ThreadCount; i++)
			{
				m_ConsumerThreadsInfo![i].Shutdown = true;
			}

			for (int i = 0; i < m_ConsPerfConfig.ThreadCount; i++)
			{
				int shutDownCount = 0;
				while (!m_ConsumerThreadsInfo![i].ShutdownAck && shutDownCount < 3) // wait for consumer thread cleanup or timeout
				{
					try
					{
						Thread.Sleep(1000);
						shutDownCount++;
					}
					catch (Exception e)
					{
						Console.WriteLine("Thread.sleep(1000) failed: {0}\n", e.Message);
					}
				}
			}
		}

		/// <summary>
		/// Stops the application
		/// </summary>
		private void Stop()
		{
			StopConsumerThreads();

			// print summary
			// If there was only one consumer thread, we didn't bother
			// gathering cross-thread stats, so copy them from the
			// lone consumer.
			if (m_ConsPerfConfig.ThreadCount == 1)
			{
				m_TotalStats = m_ConsumerThreadsInfo![0].Stats;
			}

			PrintSummaryStatistics(m_StdOutWriter);
			m_StdOutWriter.Flush();
			PrintSummaryStatistics(m_SummaryFileWriter!);
			m_SummaryFileWriter!.Close();

			for (int i = 0; i < m_ConsPerfConfig.ThreadCount; i++)
			{
				m_ConsumerThreadsInfo![i].Cleanup();
			}
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

		/// <summary>
		/// Application main method
		/// </summary>
		public void Run()
		{
			double nextTime;

			// main statistics polling thread here
			while (!_shutdownApp)
			{
				m_CurrentTime = GetTime.GetNanoseconds();
				nextTime = m_CurrentTime + 1000000000.0;

				if (!(m_ConsPerfConfig.ThreadCount > 0))
					break;

				if (m_IntervalSeconds == m_ConsPerfConfig.WriteStatsInterval)
				{
					CollectStats(true, m_ConsPerfConfig.DisplayStats, m_CurrentRuntimeSec, m_ConsPerfConfig.WriteStatsInterval);
					m_IntervalSeconds = 0;
				}

				if (m_TotalStats.ImageRetrievalEndTime > 0 && m_ConsPerfConfig.RequestSnapshots)
				{
					Console.Write("Received all images for snapshot test.\n");
					break;
				}

				if (m_CurrentTime >= m_EndTime)
				{
					if (m_TotalStats.ImageRetrievalEndTime == 0)
						Console.Write("Error: Failed to receive all images within {0} seconds.\n", m_ConsPerfConfig.SteadyStateTime);
					else
						Console.Write("\nSteady-state time of {0} seconds has expired.\n", m_ConsPerfConfig.SteadyStateTime);

					break;
				}

				try
				{
					int sleepTime = (int)(nextTime - GetTime.GetNanoseconds()) / 1000000;
					if (sleepTime > 0)
					{
						Thread.Sleep(sleepTime);
					}
					++m_CurrentRuntimeSec;
					++m_IntervalSeconds;
				}
				catch (Exception e)
				{
					Console.Write("Thread.Sleep() failed: {0}\n", e.Message);
					break;
				}
			}

			Stop();
			_runLoopExited.Signal(1);
		}

		public static void Main(string[] args)
		{
			ConsPerf consumer = new ConsPerf();
			consumer.Initialize(args);
			consumer.RegisterShutdownDelegate();
			consumer.Run();
			Environment.Exit(0);
		}

	}
}

