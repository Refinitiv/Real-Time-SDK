/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.consperf;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.util.Calendar;
import java.util.TimeZone;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.PostUserInfo;
import com.refinitiv.eta.perftools.common.PerfToolsReturnCodes;
import com.refinitiv.eta.perftools.common.ResourceUsageStats;
import com.refinitiv.eta.perftools.common.ShutdownCallback;
import com.refinitiv.eta.perftools.common.TimeRecord;
import com.refinitiv.eta.perftools.common.TimeRecordQueue;
import com.refinitiv.eta.perftools.common.ValueStatistics;
import com.refinitiv.eta.perftools.common.XmlItemInfoList;
import com.refinitiv.eta.perftools.common.XmlMsgData;

/**
 * <p>
 * The ConsPerf application. Implements a consumer, which requests items from a
 * provider and process the images and updates it receives. It also includes support
 * for posting and generic messages.
 * <p>
 * The purpose of this application is to measure performance of the ETA transport,
 * encoders and decoders in consuming Level I Market Price content directly
 * from an OMM provider or through the LSEG Real-Time Distribution System.
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
 * This application also measures memory and CPU usage. Java 7 (Oracle JDK)
 * introduced OperatingSystemMXBean which is a platform-specific management
 * interface for the operating system on which the Java virtual machine is running.
 * The getCommittedVirtualMemorySize() method is used for memory usage and the
 * getProcessCpuLoad() method is used for CPU usage.
 * <p>
 * For more detailed information on the performance measurement applications, 
 * see the ETAJ Open Source Performance Tools Guide
 * (Docs/ETAJ_PerfToolsGuide.pdf).
 * <p>
 * This application uses XML Pull Parser (XPP), an open source XML parser library.
 * <p>
 * <em>Setup Environment</em>
 * <p>
 * The following configuration files are required:
 * <ul>
 * <li>RDMFieldDictionary and enumtype.def, located in the etc directory.
 * <li>350k.xml, located in PerfTools
 * <li>MsgData.xml, located in PerfTools (only required if posting or sending generic msgs)
 * </ul>
 * <p>
 * <em>Running the application:</em>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runETAPerfConsumer -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runETAPerfConsumer -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below.
 * </p>
 * <i>-help</i> displays all command line arguments, with a brief description of each one
 */
public class ConsPerf implements ShutdownCallback
{
	private final int MAX_CONS_THREADS = 8;
	
	// post user information
	private final PostUserInfo _postUserInfo = CodecFactory.createPostUserInfo();

	// array of consumer threads
	private ConsumerThreadInfo _consumerThreadsInfo[];
	
	// application configuration information
	private final ConsPerfConfig _consPerfConfig = new ConsPerfConfig();
	
	 // item information list from XML file
	private XmlItemInfoList _xmlItemInfoList;
	
	// message data information from XML file 
	private XmlMsgData _xmlMsgData; 

	// Total statistics collected from all consumer threads. 
	// This is only used when there are multiple consumer threads. 
	private ConsumerStats _totalStats = new ConsumerStats();

	// CPU & Memory Usage samples 
	private ValueStatistics _cpuUsageStats, _memUsageStats;

	// run-time tracking  
	private long _currentTime, _endTime;
	private int _currentRuntimeSec = 0, _intervalSeconds = 0;

	// Logs summary information, such as application inputs and final statistics. 
	private File _summaryFile = null;
	private PrintWriter _summaryFileWriter;

	// stdio writer
	private PrintWriter _stdOutWriter = new PrintWriter(System.out);

	// indicates whether or not application should be shutdown
	private volatile boolean _shutdownApp = false;
	private CountDownLatch _loopExited = new CountDownLatch(1);
	private CountDownLatch _stopped = new CountDownLatch(1);

	/** Shutdown ConsPerf */
	public void shutdown()
	{
		_shutdownApp = true;
	}

	private AtomicBoolean stop = new AtomicBoolean(true);
	
	/** Run ConsPerf */
	public void run()
    {
		long nextTime;
	
		// main statistics polling thread here
		while(!_shutdownApp)
		{
			_currentTime = System.nanoTime();
			nextTime = _currentTime + 1000000000L;

			if (!(_consPerfConfig.threadCount() > 0))
				break;
			
			if (_intervalSeconds == _consPerfConfig.writeStatsInterval())
			{
				collectStats(true, _consPerfConfig.displayStats(), _currentRuntimeSec, _consPerfConfig.writeStatsInterval());
				_intervalSeconds = 0;
			}
			
			if (_totalStats.imageRetrievalEndTime() > 0 && _consPerfConfig.requestSnapshots())
			{
				System.out.println("Received all images for snapshot test.\n");
				break;
			}

			if(_currentTime >= _endTime)
			{
				if (_totalStats.imageRetrievalEndTime() == 0)
					System.out.printf("Error: Failed to receive all images within %d seconds.\n", _consPerfConfig.steadyStateTime());
				else
					System.out.printf("\nSteady-state time of %d seconds has expired.\n", _consPerfConfig.steadyStateTime());

				break;
			}

			try
			{
				long sleepTime = (nextTime-System.nanoTime())/1000000;
				if (sleepTime > 0)
				{
					Thread.sleep(sleepTime);
				}
				++_currentRuntimeSec;
				++_intervalSeconds;
			}
			catch (InterruptedException e)
			{
				System.out.printf("Thread.sleep() failed\n");
				break;
			}
		}

		_loopExited.countDown();
		if (stop.getAndSet(false)) {
			stop();
			_stopped.countDown();
		}
    }

	private void stop() {

		try {
			stopConsumerThreads();

			// print summary
			// If there was only one consumer thread, we didn't bother
			// gathering cross-thread stats, so copy them from the
			// lone consumer.
			if (_consPerfConfig.threadCount() == 1)
				_totalStats = _consumerThreadsInfo[0].stats();

			printSummaryStatistics(_stdOutWriter);
			_stdOutWriter.flush();
			printSummaryStatistics(_summaryFileWriter);
			_summaryFileWriter.close();

			for(int i = 0; i < _consPerfConfig.threadCount(); i++)
			{
				_consumerThreadsInfo[i].cleanup();
			}
		} finally {
			_stopped.countDown();
		}
	}
	
	/* Initializes ConsPerf application. */
	@SuppressWarnings("deprecation") //getId cannot be replaced for backward compatibility reasons. No equivalent method supporting both java 8 (getId()) and java >=19 (threadId()) without warnings.
	private void initialize(String[] args)
	{
	    int threadCount = 1;
	    
		// initialize and print configuration parameters
		_consPerfConfig.init(args, MAX_CONS_THREADS);
		System.out.println(_consPerfConfig.toString());
				        
        // parse item list and message data XML files 
		// the application exits if any error occured
		parseXmlFiles();
		
    	if (_consPerfConfig.postsPerSec() > 0 && _xmlItemInfoList.postItemCount() == 0)
    	{
    		System.out.printf("Error: Configured for posting but no posting items found in item file.\n");
			System.exit(-1);
    	}

    	if (_consPerfConfig.genMsgsPerSec() > 0 && _xmlItemInfoList.genMsgItemCount() == 0)
    	{
    		System.out.printf("Error: Configured for sending generic msgs but no generic msg items found in item file.\n");
			System.exit(-1);
    	}
    	
    	if (_consPerfConfig.threadCount() > 0)
    	{
    	    threadCount = _consPerfConfig.threadCount();
    	}
    	
    	_consumerThreadsInfo = new ConsumerThreadInfo[threadCount];

		// Calculate unique index for each thread. Each thread requests a common
		// and unique set of items. Unique index is so each thread has a unique
		// index into the shared item list. 
		int itemListUniqueIndex = _consPerfConfig.commonItemCount();
		for(int i = 0; i < threadCount; ++i)
		{
			_consumerThreadsInfo[i] = new ConsumerThreadInfo();
			// Figure out how many unique items each consumer should request. 
			_consumerThreadsInfo[i].itemListCount(_consPerfConfig.itemRequestCount()/threadCount);

			/* Distribute remainder. */
			if (i < _consPerfConfig.itemRequestCount() % threadCount)
				_consumerThreadsInfo[i].itemListCount(_consumerThreadsInfo[i].itemListCount() + 1);

			_consumerThreadsInfo[i].itemListUniqueIndex(itemListUniqueIndex);
			itemListUniqueIndex += (_consumerThreadsInfo[i].itemListCount() - _consPerfConfig.commonItemCount());
		}

		// create summary file writer
		_summaryFile = new File(_consPerfConfig.summaryFilename());
		try
		{
			_summaryFileWriter = new PrintWriter(_summaryFile);
		}
		catch (FileNotFoundException e)
		{
			System.out.printf("Error: Failed to open summary file '%s'.\n", _summaryFile.getName());
			System.exit(-1);
		}

		//write configuration parameters to summary file
		_summaryFileWriter.println(_consPerfConfig.toString());
		_summaryFileWriter.flush();

		//set PostUserInfo information.
		if (_consPerfConfig.postsPerSec() > 0)
		{
			try
			{
				_postUserInfo.userId(Thread.currentThread().getId());
				_postUserInfo.userAddr(InetAddress.getLocalHost().getHostAddress());
			}
			catch (Exception e)
			{
				System.out.printf("InetAddress.getLocalHost() failed while setting postUserInfo: %s\n", e.getLocalizedMessage());
				System.exit(-1);
			}
		}

		System.out.println("Starting connections(" + _consPerfConfig.threadCount() + " total)...\n");

		//Spawn consumer threads
		for(int i = 0; i < _consPerfConfig.threadCount(); ++i)
		{
			_consumerThreadsInfo[i].threadId(i + 1);
			
			new Thread(new ConsumerThread(_consumerThreadsInfo[i], _consPerfConfig, _xmlItemInfoList, _xmlMsgData, _postUserInfo, this)).start();
		}

		// set application end time
		_endTime = System.nanoTime() + _consPerfConfig.steadyStateTime() * 1000000000L;

		//create CPU and memory usage statistics
		_cpuUsageStats = new ValueStatistics();
		_memUsageStats = new ValueStatistics();		

		// Sleep for one more second so some stats can be gathered before first printout. 
		try
		{
			Thread.sleep(1000);
		}
		catch (InterruptedException e)
		{
			System.out.printf("Thread.sleep(1000) failed\n");
			System.exit(-1);
		}
	}

	
	/* Parse item list and message data XML files. */
	private void parseXmlFiles()
	{
		_xmlItemInfoList = new XmlItemInfoList(_consPerfConfig.itemRequestCount());
		_xmlMsgData = new XmlMsgData();
        if (_xmlItemInfoList.parseFile(_consPerfConfig.itemFilename()) == PerfToolsReturnCodes.FAILURE)
        {
        	System.out.printf("Failed to load item list from file '%s'.\n", _consPerfConfig.itemFilename());
			System.exit(-1);
		}
		if (_consPerfConfig.postsPerSec() > 0 || _consPerfConfig.genMsgsPerSec() > 0)
		{
	        if (_xmlMsgData.parseFile(_consPerfConfig.msgFilename()) == PerfToolsReturnCodes.FAILURE)
	        {
	        	System.out.printf("Failed to load message data from file '%s'.\n", _consPerfConfig.msgFilename());
				System.exit(-1);
			}
		}
	}

	/* Print summary statistics */
	private void printSummaryStatistics(PrintWriter fileWriter)
	{
		long firstUpdateTime;
		long totalUpdateCount = _totalStats.startupUpdateCount().getTotal()
			+ _totalStats.steadyStateUpdateCount().getTotal();

		/* Find when the first update was received. */
		firstUpdateTime = 0;
		for(int i = 0; i < _consPerfConfig.threadCount(); ++i)
		{
			if (firstUpdateTime == 0 || _consumerThreadsInfo[i].stats().firstUpdateTime() < firstUpdateTime)
				firstUpdateTime = _consumerThreadsInfo[i].stats().firstUpdateTime();
		}

		printClientSummaryStats(fileWriter);

		fileWriter.printf("\n--- OVERALL SUMMARY ---\n\n");

		fileWriter.printf("Startup State Statistics:\n");

		fileWriter.printf(
				"  Sampling duration (sec): %.3f\n",
				((_totalStats.imageRetrievalStartTime() > 0) ? 
				 ((double)((_totalStats.imageRetrievalEndTime() > 0) ? _totalStats.imageRetrievalEndTime() : _currentTime)
				- (double)(_totalStats.imageRetrievalStartTime()))/1000000000.0 : 0.0));

		if (_totalStats.startupLatencyStats().count() > 0)
		{
			fileWriter.printf("  Latency avg (usec): %.1f\n", _totalStats.startupLatencyStats().average());
			fileWriter.printf("  Latency std dev (usec): %.1f\n", Math.sqrt(_totalStats.startupLatencyStats().variance()));
			fileWriter.printf("  Latency max (usec): %.1f\n", _totalStats.startupLatencyStats().maxValue());
			fileWriter.printf("  Latency min (usec): %.1f\n", _totalStats.startupLatencyStats().minValue());
		}
		else
			fileWriter.printf("  No latency information received during startup time.\n\n");

		fileWriter.printf("  Avg update rate: %.0f\n\n", 
				_totalStats.startupUpdateCount().getTotal()
				/( (((_totalStats.imageRetrievalEndTime() > 0) ? _totalStats.imageRetrievalEndTime() : _currentTime)
						- firstUpdateTime)/1000000000.0));

		fileWriter.printf("Steady State Statistics:\n");

		if (_totalStats.imageRetrievalEndTime() > 0)
		{

			fileWriter.printf(
					"  Sampling duration (sec): %.3f\n",
					((double)_currentTime - (double)_totalStats.imageRetrievalEndTime())/1000000000.0);

			if (_totalStats.steadyStateLatencyStats().count() > 0)
			{
				fileWriter.printf("  Latency avg (usec): %.1f\n", _totalStats.steadyStateLatencyStats().average());
				fileWriter.printf("  Latency std dev (usec): %.1f\n", Math.sqrt(_totalStats.steadyStateLatencyStats().variance()));
				fileWriter.printf("  Latency max (usec): %.1f\n", _totalStats.steadyStateLatencyStats().maxValue());
				fileWriter.printf("  Latency min (usec): %.1f\n", _totalStats.steadyStateLatencyStats().minValue());
			}
			else
				fileWriter.printf("  No latency information was received during steady-state time.\n");

			if (_consPerfConfig.latencyPostsPerSec() > 0)
			{
				if (_totalStats.postLatencyStats().count() > 0)
				{
					fileWriter.printf("  Posting latency avg (usec): %.1f\n", _totalStats.postLatencyStats().average());
					fileWriter.printf("  Posting latency std dev (usec): %.1f\n", Math.sqrt(_totalStats.postLatencyStats().variance()));
					fileWriter.printf("  Posting latency max (usec): %.1f\n", _totalStats.postLatencyStats().maxValue());
					fileWriter.printf("  Posting latency min (usec): %.1f\n", _totalStats.postLatencyStats().minValue());
				}
				else
					fileWriter.printf("  No posting latency information was received during steady-state time.\n");
			}

			fileWriter.printf("  Avg update rate: %.0f\n", 
					_totalStats.steadyStateUpdateCount().getTotal()
					/((_currentTime - _totalStats.imageRetrievalEndTime())/1000000000.0));

			fileWriter.printf("\n");
		}
		else
			fileWriter.printf("  Steady state was not reached during this test.\n\n");

		fileWriter.printf("Overall Statistics: \n");

		fileWriter.printf(
				"  Sampling duration (sec): %.3f\n",
				((_totalStats.imageRetrievalStartTime() > 0) ? 
				((double)_currentTime
				 - (double)_totalStats.imageRetrievalStartTime())/1000000000.0 : 0.0));

		if (_totalStats.overallLatencyStats().count() > 0)
		{
			fileWriter.printf("  Latency avg (usec): %.1f\n", _totalStats.overallLatencyStats().average());
			fileWriter.printf("  Latency std dev (usec): %.1f\n", Math.sqrt(_totalStats.overallLatencyStats().variance()));
			fileWriter.printf("  Latency max (usec): %.1f\n", _totalStats.overallLatencyStats().maxValue());
			fileWriter.printf("  Latency min (usec): %.1f\n", _totalStats.overallLatencyStats().minValue());
		}
		else
			fileWriter.printf("  No latency information was received.\n");

		if (_totalStats.genMsgLatencyStats().count() > 0)
		{
			fileWriter.printf("  GenMsg latency avg (usec): %.1f\n", _totalStats.genMsgLatencyStats().average());
			fileWriter.printf("  GenMsg latency std dev (usec): %.1f\n", Math.sqrt(_totalStats.genMsgLatencyStats().variance()));
			fileWriter.printf("  GenMsg latency max (usec): %.1f\n", _totalStats.genMsgLatencyStats().maxValue());
			fileWriter.printf("  GenMsg latency min (usec): %.1f\n", _totalStats.genMsgLatencyStats().minValue());
		}
		else
			fileWriter.printf("  No GenMsg latency information was received.\n");

		if (_cpuUsageStats.count() > 0)
		{
			fileWriter.printf("  CPU/Memory Samples: %d\n", _cpuUsageStats.count());
			fileWriter.printf("  CPU Usage max (%%): %.2f\n", _cpuUsageStats.maxValue());
			fileWriter.printf("  CPU Usage min (%%): %.2f\n", _cpuUsageStats.minValue());
			fileWriter.printf("  CPU Usage avg (%%): %.2f\n", _cpuUsageStats.average());
			fileWriter.printf("  Memory Usage max (MB): %.2f\n", _memUsageStats.maxValue());
			fileWriter.printf("  Memory Usage min (MB): %.2f\n", _memUsageStats.minValue());
			fileWriter.printf("  Memory Usage avg (MB): %.2f\n", _memUsageStats.average());
		}
		
		fileWriter.printf("\nTest Statistics:\n");

		fileWriter.printf("  Requests sent: %d\n", _totalStats.requestCount().getTotal());
		fileWriter.printf("  Refreshes received: %d\n", _totalStats.refreshCount().getTotal());
		fileWriter.printf("  Updates received: %d\n", totalUpdateCount);

		if (_consPerfConfig.postsPerSec() > 0)
		{
			fileWriter.printf("  Posts sent: %d\n", _totalStats.postSentCount().getTotal());
		}

		if (_consPerfConfig.genMsgsPerSec() > 0)
			fileWriter.printf("  GenMsgs sent: %d\n", _totalStats.genMsgSentCount().getTotal());
		if (_totalStats.genMsgRecvCount().getTotal() > 0)
			fileWriter.printf("  GenMsgs received: %d\n", _totalStats.genMsgRecvCount().getTotal());
		if (_consPerfConfig.latencyGenMsgsPerSec() > 0)
			fileWriter.printf("  GenMsg latencies sent: %d\n", _totalStats.latencyGenMsgSentCount().getTotal());
		if (_totalStats.genMsgLatencyStats().count() > 0)
			fileWriter.printf("  GenMsg latencies received: %d\n", _totalStats.genMsgLatencyStats().count());
		
		if (_totalStats.imageRetrievalEndTime() > 0)
		{
			long totalRefreshRetrievalTime = (_totalStats.imageRetrievalEndTime() - 
					_totalStats.imageRetrievalStartTime());
			
			fileWriter.printf("  Image retrieval time (sec): %.3f\n", totalRefreshRetrievalTime/1000000000.0);
			fileWriter.printf("  Avg image Rate: %.0f\n", _consPerfConfig.itemRequestCount()/(totalRefreshRetrievalTime/1000000000.0));
		}

		fileWriter.printf("  Avg update rate: %.0f\n", 
				totalUpdateCount/((_currentTime - firstUpdateTime)/1000000000.0));

		if (_totalStats.postSentCount().getTotal() > 0)
		{
			fileWriter.printf("  Avg posting rate: %.0f\n", 
					_totalStats.postSentCount().getTotal()/((_currentTime - _totalStats.imageRetrievalEndTime())/1000000000.0));
		}

		if (_consPerfConfig.genMsgsPerSec() > 0)
		{
			fileWriter.printf("  Avg GenMsg send rate: %.0f\n", 
					_totalStats.genMsgSentCount().getTotal()/
					((_currentTime - _totalStats.imageRetrievalEndTime())/1000000000.0));
		}
		if (_totalStats.genMsgRecvCount().getTotal() > 0)
		{
			fileWriter.printf("  Avg GenMsg receive rate: %.0f\n", 
					_totalStats.genMsgRecvCount().getTotal()/
					((_currentTime - _totalStats.imageRetrievalEndTime())/1000000000.0));
		}
		if (_consPerfConfig.latencyGenMsgsPerSec() > 0)
		{
			fileWriter.printf("  Avg GenMsg latency send rate: %.0f\n", 
					_totalStats.latencyGenMsgSentCount().getTotal()/
					((_currentTime - _totalStats.imageRetrievalEndTime())/1000000000.0));
		}
		if (_totalStats.genMsgLatencyStats().count() > 0)
		{
			fileWriter.printf("  Avg GenMsg latency receive rate: %.0f\n", 
					_totalStats.genMsgLatencyStats().count()/
					((_currentTime - _totalStats.imageRetrievalEndTime())/1000000000.0));
		}
        
		fileWriter.printf("\n");
	}
	
	/* Print client summary statistics. */
	private void printClientSummaryStats(PrintWriter fileWriter)
	{
		for(int i = 0; i < _consPerfConfig.threadCount(); ++i)
		{
			long imageRetrievalTime = ((_consumerThreadsInfo[i].stats().imageRetrievalEndTime() > 0) ?
				(_consumerThreadsInfo[i].stats().imageRetrievalEndTime() 
				- _consumerThreadsInfo[i].stats().imageRetrievalStartTime()) : 0);

			/* If there are multiple connections, print individual summaries. */
			if (_consPerfConfig.threadCount() > 1)
			{
				long totalClientUpdateCount = 
					_consumerThreadsInfo[i].stats().startupUpdateCount().getTotal()
					+ _consumerThreadsInfo[i].stats().steadyStateUpdateCount().getTotal();

				fileWriter.printf("\n--- CLIENT %d SUMMARY ---\n\n", i + 1);

				fileWriter.printf("Startup State Statistics:\n");

				fileWriter.printf(
						"  Sampling duration (sec): %.3f\n",
						((_consumerThreadsInfo[i].stats().imageRetrievalStartTime() > 0) ?
						(((double)((_consumerThreadsInfo[i].stats().imageRetrievalEndTime() > 0) ? _consumerThreadsInfo[i].stats().imageRetrievalEndTime() : _currentTime)
						 - (double)_consumerThreadsInfo[i].stats().imageRetrievalStartTime())/1000000000.0) : 0.0));

				if (_consumerThreadsInfo[i].stats().startupLatencyStats().count() > 0)
				{
					fileWriter.printf("  Latency avg (usec): %.1f\n", _consumerThreadsInfo[i].stats().startupLatencyStats().average());
					fileWriter.printf("  Latency std dev (usec): %.1f\n", Math.sqrt(_consumerThreadsInfo[i].stats().startupLatencyStats().variance()));
					fileWriter.printf("  Latency max (usec): %.1f\n", _consumerThreadsInfo[i].stats().startupLatencyStats().maxValue());
					fileWriter.printf("  Latency min (usec): %.1f\n", _consumerThreadsInfo[i].stats().startupLatencyStats().minValue());
				}
				else
					fileWriter.printf("  No latency information received during startup time.\n\n");

				fileWriter.printf("  Avg update rate: %.0f\n", 
						_consumerThreadsInfo[i].stats().startupUpdateCount().getTotal()/
						((
								((_consumerThreadsInfo[i].stats().imageRetrievalEndTime() > 0) ?
								_consumerThreadsInfo[i].stats().imageRetrievalEndTime() : _currentTime)
								- _consumerThreadsInfo[i].stats().firstUpdateTime()
								)/1000000000.0));

				fileWriter.printf("\nSteady State Statistics:\n");

				if (_consumerThreadsInfo[i].stats().imageRetrievalEndTime() > 0)
				{
					fileWriter.printf(
							"  Sampling duration (sec): %.3f\n",
							((double)_currentTime - (double)_consumerThreadsInfo[i].stats().imageRetrievalEndTime())/1000000000.0);
					if (_consumerThreadsInfo[i].stats().steadyStateLatencyStats().count() > 0)
					{
						fileWriter.printf("  Latency avg (usec): %.1f\n", _consumerThreadsInfo[i].stats().steadyStateLatencyStats().average());
						fileWriter.printf("  Latency std dev (usec): %.1f\n", Math.sqrt(_consumerThreadsInfo[i].stats().steadyStateLatencyStats().variance()));
						fileWriter.printf("  Latency max (usec): %.1f\n", _consumerThreadsInfo[i].stats().steadyStateLatencyStats().maxValue());
						fileWriter.printf("  Latency min (usec): %.1f\n", _consumerThreadsInfo[i].stats().steadyStateLatencyStats().minValue());
					}
					else
						fileWriter.printf("  No latency information was received during steady-state time.\n");

					if (_consPerfConfig.latencyPostsPerSec() > 0)
					{
						if (_consumerThreadsInfo[i].stats().postLatencyStats().count() > 0)
						{
							fileWriter.printf("  Posting latency avg (usec): %.1f\n", _consumerThreadsInfo[i].stats().postLatencyStats().average());
							fileWriter.printf("  Posting latency std dev (usec): %.1f\n", Math.sqrt(_consumerThreadsInfo[i].stats().postLatencyStats().variance()));
							fileWriter.printf("  Posting latency max (usec): %.1f\n", _consumerThreadsInfo[i].stats().postLatencyStats().maxValue());
							fileWriter.printf("  Posting latency min (usec): %.1f\n", _consumerThreadsInfo[i].stats().postLatencyStats().minValue());
						}
						else
							fileWriter.printf("  No posting latency information was received during steady-state time.\n");
					}

					
					fileWriter.printf("  Avg update rate: %.0f\n", 
							_consumerThreadsInfo[i].stats().steadyStateUpdateCount().getTotal()
							/((_currentTime - _consumerThreadsInfo[i].stats().imageRetrievalEndTime())/1000000000.0));
				}
				else
					fileWriter.printf("  Steady state was not reached during this test.\n\n");

				fileWriter.printf("\nOverall Statistics: \n");

				fileWriter.printf(
						"  Sampling duration (sec): %.3f\n",
						(_consumerThreadsInfo[i].stats().imageRetrievalStartTime() > 0) ?
						((double)_currentTime
						 - (double)_consumerThreadsInfo[i].stats().imageRetrievalStartTime())/1000000000.0 : 0.0);

				if (_consumerThreadsInfo[i].stats().overallLatencyStats().count() > 0)
				{
					fileWriter.printf("  Latency avg (usec): %.1f\n", _consumerThreadsInfo[i].stats().overallLatencyStats().average());
					fileWriter.printf("  Latency std dev (usec): %.1f\n", Math.sqrt(_consumerThreadsInfo[i].stats().overallLatencyStats().variance()));
					fileWriter.printf("  Latency max (usec): %.1f\n", _consumerThreadsInfo[i].stats().overallLatencyStats().maxValue());
					fileWriter.printf("  Latency min (usec): %.1f\n", _consumerThreadsInfo[i].stats().overallLatencyStats().minValue());
				}
				else
					fileWriter.printf("  No latency information was received.\n");

				if (_consPerfConfig.latencyGenMsgsPerSec() > 0)
				{
					if (_consumerThreadsInfo[i].stats().genMsgLatencyStats().count() > 0)
					{
						fileWriter.printf("  GenMsg latency avg (usec): %.1f\n", _consumerThreadsInfo[i].stats().genMsgLatencyStats().average());
						fileWriter.printf("  GenMsg latency std dev (usec): %.1f\n", Math.sqrt(_consumerThreadsInfo[i].stats().genMsgLatencyStats().variance()));
						fileWriter.printf("  GenMsg latency max (usec): %.1f\n", _consumerThreadsInfo[i].stats().genMsgLatencyStats().maxValue());
						fileWriter.printf("  GenMsg latency min (usec): %.1f\n", _consumerThreadsInfo[i].stats().genMsgLatencyStats().minValue());
					}
					else
						fileWriter.printf("  No GenMsg latency information was received.\n");
				}

				fileWriter.printf("\nTest Statistics:\n");
				
				fileWriter.printf("  Requests sent: %d\n", _consumerThreadsInfo[i].stats().requestCount().getTotal());
				fileWriter.printf("  Refreshes received: %d\n", _consumerThreadsInfo[i].stats().refreshCount().getTotal());
				fileWriter.printf("  Updates received: %d\n", totalClientUpdateCount);

				if (_consPerfConfig.postsPerSec() > 0)
				{
					fileWriter.printf("  Posts sent: %d\n", _consumerThreadsInfo[i].stats().postSentCount().getTotal());
				}

				if (_consPerfConfig.genMsgsPerSec() > 0)
					fileWriter.printf("  GenMsgs sent: %d\n", _consumerThreadsInfo[i].stats().genMsgSentCount().getTotal());
				if (_consumerThreadsInfo[i].stats().genMsgRecvCount().getTotal() > 0)
					fileWriter.printf("  GenMsgs received: %d\n", _consumerThreadsInfo[i].stats().genMsgRecvCount().getTotal());
				if (_consPerfConfig.latencyGenMsgsPerSec() > 0)
					fileWriter.printf("  GenMsg latencies sent: %d\n", _consumerThreadsInfo[i].stats().latencyGenMsgSentCount().getTotal());
				if (_consumerThreadsInfo[i].stats().genMsgLatencyStats().count() > 0)
					fileWriter.printf("  GenMsg latencies received: %d\n", _consumerThreadsInfo[i].stats().genMsgLatencyStats().count());
				
				if (imageRetrievalTime > 0)
				{
					fileWriter.printf("  Image retrieval time(sec): %.3f\n", imageRetrievalTime/1000000000.0);
					fileWriter.printf("  Avg image Rate: %.0f\n", _consumerThreadsInfo[i].stats().refreshCount().getTotal()/(imageRetrievalTime/1000000000.0)); 
				}

				fileWriter.printf("  Avg update rate: %.0f\n",
						totalClientUpdateCount
						/((_currentTime - _consumerThreadsInfo[i].stats().firstUpdateTime())/1000000000.0));

				if (_consPerfConfig.postsPerSec() > 0)
				{
					fileWriter.printf("  Avg posting rate: %.0f\n", 
							_consumerThreadsInfo[i].stats().postSentCount().getTotal()/((_currentTime - _consumerThreadsInfo[i].stats().imageRetrievalEndTime())/1000000000.0));
				}

				if (_consPerfConfig.genMsgsPerSec() > 0)
				{
					fileWriter.printf("  Avg GenMsg send rate: %.0f\n", 
							_consumerThreadsInfo[i].stats().genMsgSentCount().getTotal()/
							((_currentTime - _consumerThreadsInfo[i].stats().imageRetrievalEndTime())/1000000000.0));
				}
				if (_consumerThreadsInfo[i].stats().genMsgRecvCount().getTotal() > 0)
				{
					fileWriter.printf("  Avg GenMsg receive rate: %.0f\n", 
							_consumerThreadsInfo[i].stats().genMsgRecvCount().getTotal()/
							((_currentTime - _consumerThreadsInfo[i].stats().imageRetrievalEndTime())/1000000000.0));
				}
				if (_consPerfConfig.latencyGenMsgsPerSec() > 0)
				{
					fileWriter.printf("  Avg GenMsg latency send rate: %.0f\n", 
							_consumerThreadsInfo[i].stats().latencyGenMsgSentCount().getTotal()/
							((_currentTime - _consumerThreadsInfo[i].stats().imageRetrievalEndTime())/1000000000.0));
				}
				if (_consumerThreadsInfo[i].stats().genMsgLatencyStats().count() > 0)
				{
					fileWriter.printf("  Avg GenMsg latency receive rate: %.0f\n", 
							_consumerThreadsInfo[i].stats().genMsgLatencyStats().count()/
							((_currentTime - _consumerThreadsInfo[i].stats().imageRetrievalEndTime())/1000000000.0));
				}
			}
		}
	}

	/* Collect statistics. */
	private void collectStats(boolean writeStats, boolean displayStats, int currentRuntimeSec, long timePassedSec) 
	{
		boolean allRefreshesRetrieved = true;
		
		double processCpuLoad = ResourceUsageStats.currentProcessCpuLoad();
		double memoryUsage = ResourceUsageStats.currentMemoryUsage();
		
		_cpuUsageStats.update(processCpuLoad);
		_memUsageStats.update(memoryUsage);

		for(int i = 0; i < _consPerfConfig.threadCount(); i++)
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
			collectUpdateStats(_consumerThreadsInfo[i]);

			/* Gather latency records for posts. */
			collectPostStats(_consumerThreadsInfo[i]);

			/* Gather latency records for generic msgs. */
			collectGenMsgStats(_consumerThreadsInfo[i]);

			if (_consumerThreadsInfo[i].latencyLogFile() != null)
				_consumerThreadsInfo[i].latencyLogFileWriter().flush();

			/* Collect counts. */
			startupUpdateCount = _consumerThreadsInfo[i].stats().startupUpdateCount().getChange();
			steadyStateUpdateCount = _consumerThreadsInfo[i].stats().steadyStateUpdateCount().getChange();
			statusCount = _consumerThreadsInfo[i].stats().statusCount().getChange();
			requestCount = _consumerThreadsInfo[i].stats().requestCount().getChange();
			refreshCount = _consumerThreadsInfo[i].stats().refreshCount().getChange();
			postOutOfBuffersCount = _consumerThreadsInfo[i].stats().genMsgOutOfBuffersCount().getChange();
			postSentCount = _consumerThreadsInfo[i].stats().postSentCount().getChange();
			genMsgSentCount = _consumerThreadsInfo[i].stats().genMsgSentCount().getChange();
            genMsgRecvCount = _consumerThreadsInfo[i].stats().genMsgRecvCount().getChange();
            latencyGenMsgSentCount = _consumerThreadsInfo[i].stats().latencyGenMsgSentCount().getChange();
            latencyGenMsgRecvCount = _consumerThreadsInfo[i].stats().intervalGenMsgLatencyStats().count();
            genMsgOutOfBuffersCount = _consumerThreadsInfo[i].stats().postOutOfBuffersCount().getChange();
 
			if (_consPerfConfig.threadCount() > 1)
			{
				_totalStats.startupUpdateCount().add(startupUpdateCount);
				_totalStats.steadyStateUpdateCount().add(steadyStateUpdateCount);
				_totalStats.statusCount().add(statusCount);
				_totalStats.requestCount().add(requestCount);
				_totalStats.refreshCount().add(refreshCount);
				_totalStats.postOutOfBuffersCount().add(postOutOfBuffersCount);
				_totalStats.postSentCount().add(postSentCount);
				_totalStats.genMsgSentCount().add(genMsgSentCount);
                _totalStats.genMsgRecvCount().add(genMsgRecvCount);
                _totalStats.latencyGenMsgSentCount().add(latencyGenMsgSentCount);
                _totalStats.genMsgOutOfBuffersCount().add(genMsgOutOfBuffersCount);
			}

			if (writeStats)
			{
				/* Log statistics to file. */
				printCurrentTimeUTC(_consumerThreadsInfo[i].statsFileWriter());
				_consumerThreadsInfo[i].statsFileWriter().printf(
						", %d, %.1f, %.1f, %.1f, %.1f, %d, %d, %d, %.1f, %.1f, %.1f, %.1f, %d, %d, %d, %d, %.1f, %.1f, %.1f, %.1f, %.2f, %.2f\n",
						_consumerThreadsInfo[i].stats().intervalLatencyStats().count(),
						_consumerThreadsInfo[i].stats().intervalLatencyStats().average(),
						Math.sqrt(_consumerThreadsInfo[i].stats().intervalLatencyStats().variance()),
						((_consumerThreadsInfo[i].stats().intervalLatencyStats().count() > 0) ? _consumerThreadsInfo[i].stats().intervalLatencyStats().maxValue() : 0.0),
						((_consumerThreadsInfo[i].stats().intervalLatencyStats().count() > 0) ? _consumerThreadsInfo[i].stats().intervalLatencyStats().minValue() : 0.0),
						refreshCount,
						(startupUpdateCount + steadyStateUpdateCount)/timePassedSec,
						_consumerThreadsInfo[i].stats().intervalPostLatencyStats().count(),
						_consumerThreadsInfo[i].stats().intervalPostLatencyStats().average(),
						Math.sqrt(_consumerThreadsInfo[i].stats().intervalPostLatencyStats().variance()),
						((_consumerThreadsInfo[i].stats().intervalPostLatencyStats().count() > 0) ? _consumerThreadsInfo[i].stats().intervalPostLatencyStats().maxValue() : 0.0),
						((_consumerThreadsInfo[i].stats().intervalPostLatencyStats().count() > 0) ? _consumerThreadsInfo[i].stats().intervalPostLatencyStats().minValue() : 0.0),
                        genMsgSentCount,
                        genMsgRecvCount,
                        latencyGenMsgSentCount,
                        latencyGenMsgRecvCount,
						_consumerThreadsInfo[i].stats().intervalGenMsgLatencyStats().average(),
						Math.sqrt(_consumerThreadsInfo[i].stats().intervalGenMsgLatencyStats().variance()),
						((_consumerThreadsInfo[i].stats().intervalGenMsgLatencyStats().count() > 0) ? _consumerThreadsInfo[i].stats().intervalGenMsgLatencyStats().maxValue() : 0.0),
						((_consumerThreadsInfo[i].stats().intervalGenMsgLatencyStats().count() > 0) ? _consumerThreadsInfo[i].stats().intervalGenMsgLatencyStats().minValue() : 0.0),
						processCpuLoad,
						memoryUsage
						);
			}

			if (displayStats)
			{
				if (_consPerfConfig.threadCount() == 1)
					System.out.printf("%03d: ", currentRuntimeSec);
				else
					System.out.printf("%03d: Client %d:\n  ", currentRuntimeSec, i + 1);

				System.out.printf("Images: %6d, Posts: %6d, UpdRate: %8d, CPU: %6.2f%%, Mem: %6.2fMB\n", 
						refreshCount,
						postSentCount,
						(startupUpdateCount + steadyStateUpdateCount)/timePassedSec,
						processCpuLoad,
						memoryUsage);

				if (_consumerThreadsInfo[i].stats().intervalLatencyStats().count() > 0)
				{
					_consumerThreadsInfo[i].stats().intervalLatencyStats().print("  Latency(usec)", "Msgs", false);
					_consumerThreadsInfo[i].stats().intervalLatencyStats().clear();
				}

				if (postOutOfBuffersCount > 0)
					System.out.printf("  - %llu posts not sent due to lack of output buffers.\n", postOutOfBuffersCount);
					

				if (_consumerThreadsInfo[i].stats().intervalPostLatencyStats().count() > 0)
				{
					_consumerThreadsInfo[i].stats().intervalPostLatencyStats().print("  PostLat(usec)", "Msgs", false);
					_consumerThreadsInfo[i].stats().intervalPostLatencyStats().clear();
				}

				if (genMsgSentCount > 0 || genMsgRecvCount > 0)
					System.out.printf("  GenMsgs: sent %d, received %d, latencies sent %d, latencies received %d\n", genMsgSentCount, genMsgRecvCount, latencyGenMsgSentCount, latencyGenMsgRecvCount);

				if (genMsgOutOfBuffersCount > 0)
					System.out.printf("  - %llu GenMsgs not sent due to lack of output buffers.\n", genMsgOutOfBuffersCount);
				
				if (_consumerThreadsInfo[i].stats().intervalGenMsgLatencyStats().count() > 0)
				{
					_consumerThreadsInfo[i].stats().intervalGenMsgLatencyStats().print("  GenMsgLat(usec)", "Msgs", false);
					_consumerThreadsInfo[i].stats().intervalGenMsgLatencyStats().clear();
				}
				
				if (_consumerThreadsInfo[i].stats().tunnelStreamBufUsageStats().count() > 0)
				{
					_consumerThreadsInfo[i].stats().tunnelStreamBufUsageStats().print("  TunnelStreamBufferUsed", "Samples", false);
					_consumerThreadsInfo[i].stats().tunnelStreamBufUsageStats().clear();
				}

				if (statusCount > 0)
					System.out.printf("  - Received %d status messages.\n", statusCount);
			}

			/* Get Image Retrieval time for this client. */
			if (getClientImageRetrievalTime(_consumerThreadsInfo[i], displayStats) == false)
			{
				allRefreshesRetrieved = false;
			}
		}
			
		if (!_totalStats.imageTimeRecorded() && allRefreshesRetrieved && _consPerfConfig.threadCount() > 0)
		{
			_endTime = _totalStats.imageRetrievalEndTime() + _consPerfConfig.steadyStateTime() * 1000000000L;
			_totalStats.imageTimeRecorded(true);

			if (_consPerfConfig.threadCount() > 1)
			{
				if (displayStats)
				{
					/* Print overall image retrieval stats. */
					long totalRefreshRetrievalTime = (_totalStats.imageRetrievalEndTime() - 
							_totalStats.imageRetrievalStartTime());

					System.out.printf("\nOverall image retrieval time for %d images: %.3fs (%.0f Images/s).\n\n", 
							_consPerfConfig.itemRequestCount(),
							totalRefreshRetrievalTime/1000000000.0,
							(_consPerfConfig.itemRequestCount())/
							(totalRefreshRetrievalTime /1000000000.0)
						  );
				}
			}
			else
			{
				_totalStats.imageRetrievalStartTime(_consumerThreadsInfo[0].stats().imageRetrievalStartTime());
				_totalStats.imageRetrievalEndTime(_consumerThreadsInfo[0].stats().imageRetrievalEndTime());
				_totalStats.steadyStateLatencyTime(_totalStats.imageRetrievalEndTime() + _consPerfConfig.delaySteadyStateCalc() * 1000000L);
			}
		}
	}

	/* Collect update statistics. */
	private void collectUpdateStats(ConsumerThreadInfo consumerThread)
	{
		TimeRecordQueue latencyRecords = consumerThread.latencyRecords();

		while (!latencyRecords.records().isEmpty())
		{
			TimeRecord record = latencyRecords.records().poll();
			double latency = (double)(record.endTime() - record.startTime())/(double)record.ticks();
			double recordEndTimeNsec = (double)record.endTime()/(double)record.ticks() * 1000.0;

			/* Make sure this latency is counted towards startup or steady-state as appropriate. */
			boolean latencyIsSteadyStateForClient = 
				consumerThread.stats().imageRetrievalEndTime() != 0
				&& recordEndTimeNsec > consumerThread.stats().imageRetrievalEndTime();

			consumerThread.stats().intervalLatencyStats().update(latency);
			consumerThread.stats().overallLatencyStats().update(latency);
			if (latencyIsSteadyStateForClient)
			{
				if (recordEndTimeNsec > consumerThread.stats().steadyStateLatencyTime())
				{
					consumerThread.stats().steadyStateLatencyStats().update(latency);
				}
			}
			else
			{
				consumerThread.stats().startupLatencyStats().update(latency);
			}

			if (_consPerfConfig.threadCount() > 1)
			{
				/* Make sure this latency is counted towards startup or steady-state as appropriate. */
				boolean latencyIsSteadyStateOverall = 
					_totalStats.imageRetrievalEndTime() != 0
					&& recordEndTimeNsec > _totalStats.imageRetrievalEndTime();

				if (latencyIsSteadyStateOverall)
				{
					if (recordEndTimeNsec > _totalStats.steadyStateLatencyTime())
					{
						_totalStats.steadyStateLatencyStats().update(latency);
					}
				}
				else
				{
					_totalStats.startupLatencyStats().update(latency);
				}
				_totalStats.overallLatencyStats().update(latency);
			}

			if (consumerThread.latencyLogFile() != null)
				consumerThread.latencyLogFileWriter().printf("Upd, %d, %d, %d\n", record.startTime(), record.endTime(), (record.endTime() - record.startTime()));
			
			latencyRecords.pool().add(record);
		}
	}
	
	/* Collect post statistics. */
	private void collectPostStats(ConsumerThreadInfo consumerThread)
	{
		TimeRecordQueue latencyRecords = consumerThread.postLatencyRecords();
		while (!latencyRecords.records().isEmpty())
		{
			TimeRecord record = latencyRecords.records().poll();
			double latency = (double)(record.endTime() - record.startTime())/(double)record.ticks();
			
			consumerThread.stats().intervalPostLatencyStats().update(latency);
			consumerThread.stats().postLatencyStats().update(latency);
			
			if (_consPerfConfig.threadCount() > 1)
				_totalStats.postLatencyStats().update(latency);

			if (consumerThread.latencyLogFile() != null)
				consumerThread.latencyLogFileWriter().printf("Pst, %d, %d, %d\n", record.startTime(), record.endTime(), (record.endTime() - record.startTime()));

			latencyRecords.pool().add(record);
		}
	}

	/* Collect generic message statistics. */
	private void collectGenMsgStats(ConsumerThreadInfo consumerThread)
	{
		TimeRecordQueue latencyRecords = consumerThread.genMsgLatencyRecords();
		while (!latencyRecords.records().isEmpty())
		{
			TimeRecord record = latencyRecords.records().poll();
			double latency = (double)(record.endTime() - record.startTime())/(double)record.ticks();
			
			consumerThread.stats().intervalGenMsgLatencyStats().update(latency);
			consumerThread.stats().genMsgLatencyStats().update(latency);
			
			if (_consPerfConfig.threadCount() > 1)
				_totalStats.genMsgLatencyStats().update(latency);

			if (consumerThread.latencyLogFile() != null)
				consumerThread.latencyLogFileWriter().printf("Gen, %d, %d, %d\n", record.startTime(), record.endTime(), (record.endTime() - record.startTime()));

			latencyRecords.pool().add(record);
		}
	}
	
	/* Get client image retrieval time. */
	private boolean getClientImageRetrievalTime(ConsumerThreadInfo consumerThread, boolean displayStats)
	{
		boolean allRefreshesRetrieved = true;
		
		if (!_totalStats.imageTimeRecorded())
		{
			if (consumerThread.stats().imageRetrievalEndTime() > 0)
			{
				long imageRetrievalStartTime = consumerThread.stats().imageRetrievalStartTime();
				long imageRetrievalEndTime = consumerThread.stats().imageRetrievalEndTime();

				/* To get the total time it took to retrieve all images, find the earliest start time
				 * and latest end time across all connections. */
				if (_totalStats.imageRetrievalStartTime() == 0 ||
						imageRetrievalStartTime < _totalStats.imageRetrievalStartTime())
					_totalStats.imageRetrievalStartTime(imageRetrievalStartTime);
				if (_totalStats.imageRetrievalEndTime() == 0 || 
						imageRetrievalEndTime > _totalStats.imageRetrievalEndTime())
					_totalStats.imageRetrievalEndTime(imageRetrievalEndTime); 
			}
			/* Ignore connections that don't request anything. */
			else if (consumerThread.itemListCount() > 0)
			{
				allRefreshesRetrieved = false; /* Not all connections have received their images yet. */
				_totalStats.imageRetrievalStartTime(0);
				_totalStats.imageRetrievalEndTime(0);
			}
		}

		if (!consumerThread.stats().imageTimeRecorded() && consumerThread.stats().imageRetrievalEndTime() > 0)
		{
			consumerThread.stats().imageTimeRecorded(true);

			if (displayStats)
			{
				long imageRetrievalTime = consumerThread.stats().imageRetrievalEndTime() - 
					consumerThread.stats().imageRetrievalStartTime();

				System.out.printf("  - Image retrieval time for %d images: %.3fs (%.0f images/s)\n", 
						consumerThread.itemListCount(),
						imageRetrievalTime/1000000000.0,
						(consumerThread.itemListCount())/
						(imageRetrievalTime /1000000000.0));
			}
		}
		
		return allRefreshesRetrieved;
	}

	/* Stop all consumer threads. */
	private void stopConsumerThreads()
	{
		for(int i = 0; i < _consPerfConfig.threadCount(); i++)
		{
			_consumerThreadsInfo[i].shutdown(true);
		}

		for(int i = 0; i < _consPerfConfig.threadCount(); i++)
		{
			int shutDownCount = 0;
			// wait for consumer thread cleanup or timeout
			while (!_consumerThreadsInfo[i].shutdownAck() && shutDownCount < 3)
			{
				try
				{
					Thread.sleep(1000);
					shutDownCount++;
				}
				catch (InterruptedException e)
				{
					System.out.printf("Thread.sleep(1000) failed\n");
				}
			}
		}
	}

	/* Prints the current time, in Coordinated Universal Time. */
	private void printCurrentTimeUTC(PrintWriter fileWriter)
	{
        Calendar rightNow = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
		fileWriter.printf("%d-%02d-%02d %02d:%02d:%02d",
				rightNow.get(Calendar.YEAR), rightNow.get(Calendar.MONTH), rightNow.get(Calendar.DAY_OF_MONTH),
				rightNow.get(Calendar.HOUR_OF_DAY), rightNow.get(Calendar.MINUTE), rightNow.get(Calendar.SECOND));
	}

	private void registerShutdownHook() {
		Runtime.getRuntime().addShutdownHook(new Thread(() -> {
			try {
				if (stop.getAndSet(false)) {
					shutdown();
					_loopExited.await(1000, TimeUnit.MILLISECONDS);
					stop();
				} else {
					_stopped.await(_consPerfConfig.threadCount() * 5000, TimeUnit.MILLISECONDS);
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}));
	}

	public static void main(String[] args)
	{
		ConsPerf consumerperf = new ConsPerf();
		consumerperf.initialize(args);
		consumerperf.registerShutdownHook();
		consumerperf.run();
		System.exit(0);
	}
}

