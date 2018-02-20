package com.thomsonreuters.upa.perftools.common;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.util.Calendar;
import java.util.TimeZone;

import com.thomsonreuters.upa.perftools.upajniprovperf.NIProviderThread;
import com.thomsonreuters.upa.perftools.upajprovperf.IProviderThread;

/**
 * Maintains the provider application instance. Has logic for keeping track of
 * things like CPU/Mem usage, and the list of provider threads.
 */
public class Provider 
{
    
	private PrintWriter _summaryFileWriter;      // Logs summary information, such as application
                                           // inputs and final statistics.
	private File _summaryFile = null;            // summary file

    private ProviderThread[]         _providerThreadList;                // Provider threads
    private ProviderType             _providerType;                      // Type of provider.  
    private ProviderThreadStats      _totalStats;                        // only used for multiple provider threads
  
    private ValueStatistics          _cpuUsageStats;                     // Sampled CPU statistics. 
    private ValueStatistics          _memUsageStats;                     // Sampled memory usage statistics.
    
    private CountStat                _refreshCount;                      // Counts refreshes sent (for all provider threads).
    private CountStat                _updateCount;                       // Counts updates sent (for all provider threads).
    private CountStat                _requestCount;                      // Counts requests received (for all provider threads).
    private CountStat                _closeCount;                        // Counts closes received (for all provider threads).
    private CountStat                _postCount;                         // Counts posts received (for all provider threads).
    private CountStat                _genMsgSentCount;                   // Counts generic msgs sent (for all provider threads).
    private CountStat                _genMsgRecvCount;                   // Counts generic msgs received (for all provider threads).
    private CountStat                _latencyGenMsgSentCount;            // Counts latency generic msgs sent (for all provider threads). 
    private CountStat                _outOfBuffersCount;                 // Counts updates not sent due to lack
                                                                         // of output buffers.
    private CountStat                _msgSentCount;                      // Counts total messages sent.
    private CountStat                _bufferSentCount;                   // Counts total buffers sent(used with
                                                                         // msgSentCount for packing statistics).

    /**
                                                                          * Instantiates a new provider.
                                                                          */
                                                                         public Provider()
    {
        _refreshCount = new CountStat();
        _updateCount = new CountStat();
        _requestCount = new CountStat();
        _closeCount = new CountStat();
        _postCount = new CountStat();
        _genMsgSentCount = new CountStat();
        _genMsgRecvCount = new CountStat();
        _latencyGenMsgSentCount = new CountStat();
        _outOfBuffersCount = new CountStat();
        _msgSentCount = new CountStat();
        _bufferSentCount = new CountStat();
        _cpuUsageStats = new ValueStatistics();
        _memUsageStats = new ValueStatistics();
        _totalStats = new ProviderThreadStats();
    }
    
    /**
     * Initializes provider statistics and creates specific number provider
     * threads as configured by the application.
     *
     * @param providerType the provider type
     * @param xmlMsgData the xml msg data
     * @param summaryFileName the summary file name
     */
    public void init(ProviderType providerType, XmlMsgData xmlMsgData, String summaryFileName)
    {
        _providerType = providerType;
        _refreshCount.init();
        _updateCount.init();
        _requestCount.init();
        _closeCount.init();
        _postCount.init();
        _genMsgSentCount.init();
        _genMsgRecvCount.init();
        _latencyGenMsgSentCount.init();
        _outOfBuffersCount.init();
        _msgSentCount.init();
        _bufferSentCount.init();
        _cpuUsageStats.clear();
        _memUsageStats.clear();
        
        _providerThreadList = new ProviderThread[ProviderPerfConfig.threadCount()];
        
        for (int i = 0; i < ProviderPerfConfig.threadCount(); i++)
        {
        	if (providerType == ProviderType.PROVIDER_INTERACTIVE)
        	{
        		_providerThreadList[i] = new IProviderThread(xmlMsgData);
        	}
        	else if (providerType == ProviderType.PROVIDER_NONINTERACTIVE)
        	{
        		_providerThreadList[i] = new NIProviderThread(xmlMsgData);        		
        	}

            _providerThreadList[i].init(i, providerType);
        }

        _summaryFile = new File(summaryFileName);
        try
        {
            _summaryFileWriter = new PrintWriter(_summaryFile);
        }
        catch (FileNotFoundException e)
        {
            System.out.printf("Error: Failed to open summary file '%s'.\n", _summaryFile.getName());
            System.exit(-1);
        }

        if (providerType == ProviderType.PROVIDER_INTERACTIVE)
        {
        	_summaryFileWriter.println(ProviderPerfConfig.convertToString());
        }
        else if (providerType == ProviderType.PROVIDER_NONINTERACTIVE)
        {
        	_summaryFileWriter.println(NIProvPerfConfig.convertToString());        	
        }
        _summaryFileWriter.flush();

    }
    
    /**
     * Starts provider threads.
     */
    public void startThreads()
    {
        for (ProviderThread providerThread : _providerThreadList)
        {
            providerThread.start();
        }
    }
    
    /**
     * Clean up provider threads.
     */
    public void cleanup()
    {
        for(ProviderThread provideThread : _providerThreadList)
        {
            provideThread.cleanup();
        }
        
        waitForThreads();
    }

    /* Collect generic message statistics. */
    private void collectGenMsgStats(ProviderThreadInfo provThreadInfo)
    {
        TimeRecordQueue latencyRecords = provThreadInfo.genMsgLatencyRecords();
        while (!latencyRecords.records().isEmpty())
        {
            TimeRecord record = latencyRecords.records().poll();
            double latency = (double)(record.endTime() - record.startTime())/(double)record.ticks();
            
            provThreadInfo.stats().intervalGenMsgLatencyStats().update(latency);
            provThreadInfo.stats().genMsgLatencyStats().update(latency);
            
            if (ProviderPerfConfig.threadCount() > 1)
                _totalStats.genMsgLatencyStats().update(latency);

            if (provThreadInfo.latencyLogFile() != null)
                provThreadInfo.latencyLogFileWriter().printf("Gen, %d, %d, %d\n", record.startTime(), record.endTime(), (record.endTime() - record.startTime()));

            latencyRecords.pool().add(record);
        }
    }
    
    /**
     * Collects and writes provider statistics. Stats will reflect changes from the previous
     * call to this method.
     * 
     * @param writeStats - if true, writes statistics to provider stats file
     * @param displayStats - if true, writes stats to stdout. 
     * @param currentRuntimeSec - current time
     * @param timePassedSec - time passed since last stats collection, used to calculate message rates.
     */
    public void collectStats(boolean writeStats, boolean displayStats, long currentRuntimeSec, long timePassedSec)
    {
        long refreshCount, updateCount, requestCount, closeCount, postCount, genMsgSentCount, genMsgRecvCount, latencyGenMsgSentCount, latencyGenMsgRecvCount, outOfBuffersCount, msgSentCount, bufferSentCount;
        double processCpuLoad = ResourceUsageStats.currentProcessCpuLoad();
        double memoryUsage = ResourceUsageStats.currentMemoryUsage();
        if(timePassedSec != 0)
        {
            _cpuUsageStats.update(processCpuLoad);
            _memUsageStats.update(memoryUsage);
        }

        for(int i = 0; i < ProviderPerfConfig.threadCount(); ++i)
        {
            ProviderThread providerThread = providerThreadList()[i];
            collectGenMsgStats(providerThread.getProvThreadInfo());
            
            if (providerThread.getProvThreadInfo().latencyLogFileWriter() != null)
            {
                providerThread.getProvThreadInfo().latencyLogFileWriter().flush();
            }
            ProviderThreadStats stats = providerThread.getProvThreadInfo().stats();

            requestCount = providerThread.itemRequestCount().getChange();
            refreshCount = providerThread.refreshMsgCount().getChange();
            updateCount = providerThread.updateMsgCount().getChange();
            closeCount = providerThread.closeMsgCount().getChange();
            postCount = providerThread.postMsgCount().getChange();
            genMsgSentCount = stats.genMsgSentCount().getChange();
            genMsgRecvCount = stats.genMsgRecvCount().getChange();
            latencyGenMsgSentCount = stats.latencyGenMsgSentCount().getChange();
            latencyGenMsgRecvCount = stats.intervalGenMsgLatencyStats().count();
            outOfBuffersCount = providerThread.outOfBuffersCount().getChange();

            if(writeStats)
            {
                // Write stats to the stats file.
                Calendar rightNow = Calendar.getInstance(TimeZone.getTimeZone("GMT"));

                providerThread.getProvThreadInfo().statsFileWriter().printf("%d-%02d-%02d %02d:%02d:%02d",
        				rightNow.get(Calendar.YEAR), rightNow.get(Calendar.MONTH), rightNow.get(Calendar.DAY_OF_MONTH),
        				rightNow.get(Calendar.HOUR_OF_DAY), rightNow.get(Calendar.MINUTE), rightNow.get(Calendar.SECOND));

                switch(_providerType)
                {
                    case PROVIDER_INTERACTIVE:
                        providerThread.getProvThreadInfo().statsFileWriter().printf(
                        		", %d, %d, %d, %d, %d, %d, %d, %d, %.1f, %.1f, %.1f, %.1f, %.2f, %.2f\n", 
                        		requestCount,
                                refreshCount,
                                updateCount,
                                postCount,
                                genMsgSentCount,
                                genMsgRecvCount,
                                latencyGenMsgSentCount,
                                latencyGenMsgRecvCount,
                                stats.intervalGenMsgLatencyStats().average(),
                                Math.sqrt(stats.intervalGenMsgLatencyStats().variance()),
                                ((stats.intervalGenMsgLatencyStats().count() > 0) ? stats.intervalGenMsgLatencyStats().maxValue() : 0.0),
                                ((stats.intervalGenMsgLatencyStats().count() > 0) ? stats.intervalGenMsgLatencyStats().minValue() : 0.0),
                                processCpuLoad,
                                memoryUsage);
                        break;
                    case PROVIDER_NONINTERACTIVE:
                        providerThread.getProvThreadInfo().statsFileWriter().printf(
                        		", %d, %d, %d, %.2f, %.2f\n", 
                        		requestCount,
                                refreshCount,
                                updateCount,
                                processCpuLoad,
                                memoryUsage);
                        break;
                }
                providerThread.getProvThreadInfo().statsFileWriter().flush();
            }

            //Add the new counts to the provider's total.
            _requestCount.add(requestCount);
            _updateCount.add(updateCount);
            _refreshCount.add(refreshCount);
            _closeCount.add(closeCount);
            _postCount.add(postCount);
            _genMsgSentCount.add(genMsgSentCount);
            _genMsgRecvCount.add(genMsgRecvCount);
            _latencyGenMsgSentCount.add(latencyGenMsgSentCount);
            _outOfBuffersCount.add(outOfBuffersCount);

            //Take packing stats, if packing is enabled.
            if(ProviderPerfConfig.totalBuffersPerPack() > 1)
            {
                _msgSentCount.add(providerThread.msgSentCount().getChange());
                _bufferSentCount.add(providerThread.bufferSentCount().getChange());
            }

            if(displayStats)
            {
                //Print screen stats.
                if(ProviderPerfConfig.threadCount() == 1)
                    System.out.printf("%03d: ", currentRuntimeSec);
                else
                    System.out.printf("%03d: Thread %d:\n  ", currentRuntimeSec, i + 1);

                System.out.printf("UpdRate: %8d, CPU: %6.2f%%, Mem: %6.2fMB\n", 
                                  updateCount/timePassedSec,
                                  processCpuLoad, memoryUsage);

                switch(_providerType)
                {
                    case PROVIDER_INTERACTIVE:
                        if(requestCount > 0 || refreshCount > 0)
                            System.out.printf("  - Received %d item requests (total: %d), sent %d images (total: %d)\n",
                                              requestCount,
                                              _requestCount.getTotal(),
                                              refreshCount,
                                              _refreshCount.getTotal());
                        if(postCount > 0)
                            System.out.printf("  Posting: received %d, reflected %d\n", postCount, postCount);
                        if(genMsgRecvCount > 0 || genMsgSentCount > 0)
                        {
                            System.out.printf("  GenMsgs: sent %d, received %d, latencies sent %d, latencies received %d\n", 
                            		genMsgSentCount, genMsgRecvCount, latencyGenMsgSentCount, latencyGenMsgRecvCount);
                        }
                        if (stats.intervalGenMsgLatencyStats().count() > 0)
                        {
                            stats.intervalGenMsgLatencyStats().print("  GenMsgLat(usec)", "Msgs", false);
                            stats.intervalGenMsgLatencyStats().clear();
                        }
                        break;
                    case PROVIDER_NONINTERACTIVE:
                        if (requestCount > 0 || refreshCount > 0)
                            System.out.printf("  - Sent %d images (total: %d)\n", refreshCount, _refreshCount.getTotal());
                        break;
                }

                closeCount = _closeCount.getChange();
                if (closeCount > 0)
                    System.out.printf("  - Received %d closes.\n", closeCount);

                outOfBuffersCount = _outOfBuffersCount.getChange();

                if (outOfBuffersCount > 0)
                    System.out.printf("  - Stopped %d updates due to lack of output buffers.\n", outOfBuffersCount);

                // Print packing stats, if packing is enabled. 
                msgSentCount =_msgSentCount.getChange();
                bufferSentCount = _bufferSentCount.getChange();
                if (bufferSentCount > 0)
                {
                    System.out.printf("  - Approx. avg msgs per pack: %.0f\n", (double)msgSentCount/(double)bufferSentCount);
                }
            }
        }
    }
    
    /**
     * Prints summary stats to the summary file.
     * 
     * @param fileWriter  - file to write summary stats to.
     */
    public void printSummaryStats(PrintWriter fileWriter)
    {
    	long statsTime = 0;
    	long currentTime = System.nanoTime();

    	if(ProviderPerfConfig.threadCount() > 1)
        {
    		_totalStats.inactiveTime(_providerThreadList[0].getProvThreadInfo().stats().inactiveTime());
        	_totalStats.firstGenMsgSentTime(_providerThreadList[0].getProvThreadInfo().stats().firstGenMsgSentTime());
        	_totalStats.firstGenMsgRecvTime(_providerThreadList[0].getProvThreadInfo().stats().firstGenMsgRecvTime());
            for(int i = 0; i < ProviderPerfConfig.threadCount(); ++i)
            {
                ProviderThread providerThread = _providerThreadList[i];
                ProviderThreadStats stats = providerThread.getProvThreadInfo().stats();
                statsTime = (stats.inactiveTime() > 0 && stats.inactiveTime() < currentTime) ? stats.inactiveTime() : currentTime;
                if(stats.inactiveTime() > 0 && stats.inactiveTime() < _totalStats.inactiveTime())
                	_totalStats.inactiveTime(stats.inactiveTime());
                if(stats.firstGenMsgSentTime() !=0 && stats.firstGenMsgSentTime() < _totalStats.firstGenMsgSentTime())
                	_totalStats.firstGenMsgSentTime(stats.firstGenMsgSentTime());
                if(stats.firstGenMsgRecvTime() !=0 && stats.firstGenMsgRecvTime() < _totalStats.firstGenMsgRecvTime())
                	_totalStats.firstGenMsgSentTime(stats.firstGenMsgRecvTime());
                fileWriter.printf("\n--- THREAD %d SUMMARY ---\n\n", i + 1);

                fileWriter.printf("Overall Statistics: \n");

                switch(_providerType)
                {
                    case PROVIDER_INTERACTIVE:
                        if(_totalStats.genMsgLatencyStats().count() > 0)
                        {
                            fileWriter.printf("  GenMsg latency avg (usec): %.1f\n" +
                                              "  GenMsg latency std dev (usec): %.1f\n" +
                                              "  GenMsg latency max (usec): %.1f\n" +
                                              "  GenMsg latency min (usec): %.1f\n",
                                              stats.genMsgLatencyStats().average(),
                                              Math.sqrt(stats.genMsgLatencyStats().variance()),
                                              stats.genMsgLatencyStats().count() > 0 ? stats.genMsgLatencyStats().maxValue() : 0,
                                              stats.genMsgLatencyStats().count() > 0 ? stats.genMsgLatencyStats().minValue() : 0);
                        }
                        else
                            fileWriter.printf("  No GenMsg latency information was received.\n");
                        if (ProviderPerfConfig.genMsgsPerSec() > 0)
                        	fileWriter.printf("  GenMsgs sent: %d\n", stats.genMsgSentCount().getTotal());
                        if (stats.genMsgRecvCount().getTotal() > 0)
                        	fileWriter.printf("  GenMsgs received: %d\n", stats.genMsgRecvCount().getTotal());
                        if (ProviderPerfConfig.latencyGenMsgRate() > 0)
                        	fileWriter.printf("  GenMsg latencies sent: %d\n", stats.latencyGenMsgSentCount().getTotal());
                        if (stats.genMsgLatencyStats().count() > 0)
                        	fileWriter.printf("  GenMsg latencies received: %d\n", stats.genMsgLatencyStats().count());  
                        if (ProviderPerfConfig.genMsgsPerSec() > 0)
                        {
                        	fileWriter.printf("  Avg GenMsg send rate: %.0f\n", stats.genMsgSentCount().getTotal()/
                        			(double)((statsTime - stats.firstGenMsgSentTime())/1000000000.0));
                        }
                        if (stats.genMsgRecvCount().getTotal() > 0)
                        {
                        	fileWriter.printf("  Avg GenMsg receive rate: %.0f\n", stats.genMsgRecvCount().getTotal()/
                        			(double)((statsTime - stats.firstGenMsgRecvTime())/1000000000.0));
                        }
                        if (ProviderPerfConfig.latencyGenMsgRate() > 0)
                        {
                        	fileWriter.printf("  Avg GenMsg latency send rate: %.0f\n", stats.latencyGenMsgSentCount().getTotal()/
                        			(double)((statsTime - stats.firstGenMsgSentTime())/1000000000.0));
                        }
                        if (stats.genMsgLatencyStats().count() > 0)
                        {
                        	fileWriter.printf("  Avg GenMsg latency receive rate: %.0f\n", stats.genMsgLatencyStats().count()/
                        			(double)((statsTime - stats.firstGenMsgRecvTime())/1000000000.0));
                        }
                        fileWriter.printf("  Image requests received: %d\n", providerThread.itemRequestCount().getTotal());
                        if (ProviderPerfConfig.updatesPerSec() > 0)
                        	fileWriter.printf("  Updates sent: %d\n", providerThread.updateMsgCount().getTotal());
                        if (providerThread.postMsgCount().getTotal() > 0)
                        {
                        	fileWriter.printf("  Posts received: %d\n", providerThread.postMsgCount().getTotal()); 
                        	fileWriter.printf("  Posts reflected: %d\n", providerThread.postMsgCount().getTotal());
                        }
                        break;
                    case PROVIDER_NONINTERACTIVE:
                        fileWriter.printf(
                                "  Images sent: %d\n" +
                                "  Updates sent: %d\n",
                                providerThread.refreshMsgCount().getTotal(),
                                providerThread.updateMsgCount().getTotal());
                        break;
                }
            }
        }
        else
        {
        	_totalStats = _providerThreadList[0].getProvThreadInfo().stats();
        	statsTime = (_totalStats.inactiveTime() != 0 && _totalStats.inactiveTime() < currentTime) ? _totalStats.inactiveTime() : currentTime;
        }
        fileWriter.printf("\n--- OVERALL SUMMARY ---\n\n");

        fileWriter.printf("Overall Statistics: \n");
       
        switch(_providerType)
        {
            case PROVIDER_INTERACTIVE:
                if(_totalStats.genMsgLatencyStats().count() > 0)
                {
                    fileWriter.printf("  GenMsg latency avg (usec): %.1f\n" +
                                      "  GenMsg latency std dev (usec): %.1f\n" +
                                      "  GenMsg latency max (usec): %.1f\n" +
                                      "  GenMsg latency min (usec): %.1f\n",
                                      _totalStats.genMsgLatencyStats().average(),
                                      Math.sqrt(_totalStats.genMsgLatencyStats().variance()),
                                      _totalStats.genMsgLatencyStats().maxValue(),
                                      _totalStats.genMsgLatencyStats().minValue());
                }
                else
                    fileWriter.printf("  No GenMsg latency information was received.\n");
                if (ProviderPerfConfig.genMsgsPerSec() > 0)
                	fileWriter.printf("  GenMsgs sent: %d\n", _genMsgSentCount.getTotal());
                if (_genMsgRecvCount.getTotal() > 0)
                	fileWriter.printf("  GenMsgs received: %d\n", _genMsgRecvCount.getTotal());
                if (ProviderPerfConfig.latencyGenMsgRate() > 0)
                	fileWriter.printf("  GenMsg latencies sent: %d\n", _latencyGenMsgSentCount.getTotal());
                if (_totalStats.genMsgLatencyStats().count() > 0)
                	fileWriter.printf("  GenMsg latencies received: %d\n", _totalStats.genMsgLatencyStats().count()); 
                if (ProviderPerfConfig.genMsgsPerSec() > 0)
                {
                	fileWriter.printf("  Avg GenMsg send rate: %.0f\n", _genMsgSentCount.getTotal()/
                			(double)((statsTime - _totalStats.firstGenMsgSentTime())/1000000000.0));
                }
                if (_genMsgRecvCount.getTotal() > 0)
                {
                	fileWriter.printf("  Avg GenMsg receive rate: %.0f\n", _genMsgRecvCount.getTotal()/
                			(double)((statsTime - _totalStats.firstGenMsgRecvTime())/1000000000.0));
                }
                if (ProviderPerfConfig.latencyGenMsgRate() > 0)
                {
                	fileWriter.printf("  Avg GenMsg latency send rate: %.0f\n", _latencyGenMsgSentCount.getTotal()/
                			(double)((statsTime - _totalStats.firstGenMsgSentTime())/1000000000.0));
                }
                if (_totalStats.genMsgLatencyStats().count() > 0)
                {
                	fileWriter.printf("  Avg GenMsg latency receive rate: %.0f\n", _totalStats.genMsgLatencyStats().count()/
                			(double)((statsTime - _totalStats.firstGenMsgRecvTime())/1000000000.0));
                }
                fileWriter.printf("  Image requests received: %d\n", _requestCount.getTotal());
                if (ProviderPerfConfig.updatesPerSec() > 0)
                	fileWriter.printf("  Updates sent: %d\n", _updateCount.getTotal());
                if (_postCount.getTotal() > 0)
                {
                	fileWriter.printf("  Posts received: %d\n", _postCount.getTotal()); 
                	fileWriter.printf("  Posts reflected: %d\n", _postCount.getTotal());
                }
                break;
            case PROVIDER_NONINTERACTIVE:
                fileWriter.printf("  Image sent: %d\n" +
                        "  Updates sent: %d\n",
                        _refreshCount.getTotal(),
                        _updateCount.getTotal());
                break;
        }
        
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
        
        fileWriter.printf("\n");
    }
    

    /**
     * Prints the final stats.
     */
    public void printFinalStats()
    {
    	PrintWriter printWriter = new PrintWriter(System.out);
    	printSummaryStats(printWriter);
    	printWriter.flush();
    	printSummaryStats(_summaryFileWriter);
    	_summaryFileWriter.close();
    }
    /*
     * Wait for provider threads to die.
     */
    private void waitForThreads()
    {
        for(int i = 0; i < _providerThreadList.length; ++i)
        {
            try
            {
                _providerThreadList[i].join(1000);
            }
            catch (InterruptedException e)
            {
                //
            }
        }
        
        _providerThreadList = null;
    }

    /**
     * Provider threads.
     * 
     * @return provider threads
     */
    public ProviderThread[] providerThreadList()
    {
        return _providerThreadList;
    }
}
