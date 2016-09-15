package com.thomsonreuters.upa.perftools.upajtransportperf;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.perftools.common.ClientChannelInfo;
import com.thomsonreuters.upa.perftools.common.ResourceUsageStats;
import com.thomsonreuters.upa.perftools.common.ShutdownCallback;
import com.thomsonreuters.upa.perftools.common.TimeRecord;
import com.thomsonreuters.upa.perftools.common.TimeRecordQueue;
import com.thomsonreuters.upa.perftools.common.ValueStatistics;
import com.thomsonreuters.upa.transport.AcceptOptions;
import com.thomsonreuters.upa.transport.BindOptions;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InitArgs;
import com.thomsonreuters.upa.transport.Server;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

/**
 * The upajTransportPerf application. This application may act as a client or
 * server as appropriate, and tests the sending of raw messages across connections.
 * <p>
 * <H2>Summary</H2>
 * <p>
 * The purpose of this application is to measure performance of the UPA transport,
 * using the different supported connection types with variable message sizes.
 * <p>
 * The content provided by this application is intended for raw transport
 * measurement and does not use the OMM encoders and decoders. The message
 * contains only a sequence number and random timestamp. The remainder of
 * the message is padded with zeros.
 * <p>
 * The application creates two types of threads:
 * <ul>
 * <li>A main thread, which collects and records statistical information.
 * <li>Connection threads, each of which connect or accept connections, and
 * pass messages across.
 * </ul>
 * <p>
 * To measure latency, a timestamp is randomly placed in each burst of messages
 * sent. The receiver of the messages reads the timestamp and compares it to the
 * current time to determine the end-to-end latency.
 * <p>
 * This application also measures memory and CPU usage. Java 7 (Oracle JDK)
 * introduced OperatingSystemMXBean which is a platform-specific management
 * interface for the operating system on which the Java virtual machine is
 * running. The getCommittedVirtualMemorySize() method is used for memory usage
 * and the getProcessCpuLoad() method is used for CPU usage.
 * <p>
 * For more detailed information on the performance measurement applications,
 * see the UPA-J Open Source Performance Tools Guide
 * (Docs/UPAJPerfToolsGuide.pdf).
 * <p>
 * <H2>Setup Environment</H2>
 * <p>
 * No additional files are necessary to run this application.
 * <p>
 * <H2>Running the application:</H2>
 * From the <i>Applications/PerfTools</i> directory run <i>ant</i> or <i>buildPerfTools.bat/ksh</i> script to
 * build performance examples.
 * <p>
 * Run example with the following command:
 * <p>
 * java -cp ../../Libs/upa.jar;../../Libs/upaValueAdd.jar;./xpp3-1.1.3_8.jar;./xpp3_min-1.1.3_8.jar;bin
 * com.thomsonreuters.upa.perftools.upajtransportperf.upajTransportPerf
 * <p>
 * <i>-help</i> displays command line options, with a brief description of each option
 * <p>
 */
public class upajTransportPerf implements ShutdownCallback
{   
    private Error _error; /* error information */
    private int _sessionHandlerCount;
    private SessionHandler[] _sessionHandlerList;
    private ProcessMsg _processMsg;
    private InitArgs _initArgs; /* arguments for initializing transport */
    private boolean _runTimeExpired; /* application run-time expired */
    private Server _srvr; // used if we do a Transport.bind
    private Selector _selector; // selector
    private AcceptOptions _acceptOptions; // server accept options
    
    /* CPU & Memory Usage samples */
    private ValueStatistics _cpuUsageStats, _memUsageStats;
    
    private ValueStatistics _totalLatencyStats, _intervalLatencyStats;
    
    private long _totalMsgSentCount = 0;
    private long _totalBytesSent = 0;
    private long _totalMsgReceivedCount = 0;
    private long _totalBytesReceived = 0;
    
    /* Logs summary information, such as application inputs and final statistics. */
    private File _summaryFile = null;
    private PrintWriter _summaryFileWriter;
    private PrintWriter _stdOutWriter; /* stdio writer */ 

    private DateTime _dateTime; /* storage for GMT time */
    
    /* indicates whether or not application should be shutdown */
    private volatile boolean _shutdownApp = false;
    
    private boolean negativeLatencyFound; // negative latency found flag

    {
        _error = TransportFactory.createError();
        _initArgs = TransportFactory.createInitArgs();
        _cpuUsageStats = new ValueStatistics();
        _memUsageStats = new ValueStatistics();
        _totalLatencyStats = new ValueStatistics();
        _intervalLatencyStats = new ValueStatistics();
        _acceptOptions = TransportFactory.createAcceptOptions();
        _stdOutWriter = new PrintWriter(System.out);
        _dateTime = CodecFactory.createDateTime();
        try
        {
            _selector = Selector.open();
        }
        catch (Exception exception)
        {
            System.err.println("Error initializing application: Unable to open selector: " + exception.getMessage());
            System.exit(-1);
        }
    }

    /** Shutdown upajTransportPerf */
    public void shutdown()
    {       
        _shutdownApp = true;
        
    }
    
    /* Initializes upajTransportPerf application. */
    private void initialize(String[] args)
    {
        /* Read in configuration and echo it */
        TransportPerfConfig.init(args);
        System.out.println(TransportPerfConfig.configString());

        if (TransportPerfConfig.appType() == TransportPerfConfig.SERVER && 
                (TransportPerfConfig.connectionType() == ConnectionTypes.HTTP || 
                TransportPerfConfig.connectionType() == ConnectionTypes.ENCRYPTED ||
                TransportPerfConfig.connectionType() == ConnectionTypes.SEQUENCED_MCAST))
        {
            System.out.printf("Error: Does not support http, encypted, or sequenced multicast connectionType while running as server.");
            System.exit(-1);            
        }
                                
        /* Create summary file writer */
        _summaryFile = new File(TransportPerfConfig.summaryFilename());
        try
        {
            _summaryFileWriter = new PrintWriter(_summaryFile);
        }
        catch (FileNotFoundException e)
        {
            System.out.printf("Error: Failed to open summary file '%s'.\n", _summaryFile.getName());
            System.exit(-1);
        }

        /* Write configuration parameters to summary file */
        _summaryFileWriter.println(TransportPerfConfig.configString());
        _summaryFileWriter.flush();
        
        _sessionHandlerCount = TransportPerfConfig.threadCount();
        _sessionHandlerList = new SessionHandler[_sessionHandlerCount];
        for (int i = 0; i < _sessionHandlerCount; i++)
        {
            _sessionHandlerList[i] = new SessionHandler();
        }
        
        /* Initialize UPA */
        _initArgs.clear();
        _initArgs.globalLocking(TransportPerfConfig.threadCount() > 1 ? true : false);
        if (Transport.initialize(_initArgs, _error) != TransportReturnCodes.SUCCESS)
        {
            System.err.println("Error: Transport failed to initialize: " + _error.text());
            System.exit(-1);
        }

        /* Spawn threads */
        for (int i = 0; i < _sessionHandlerCount; ++i)
        {
            _sessionHandlerList[i].init();
            if(TransportPerfConfig.reflectMsgs())
            {
                _sessionHandlerList[i].role(TransportTestRole.REFLECTOR);
                _processMsg = new ProcessMsgReflect();
            }
            else
            {
                _sessionHandlerList[i].role(TransportTestRole.WRITER | TransportTestRole.READER);
                _processMsg = new ProcessMsgReadWrite();
            }
            _sessionHandlerList[i].active(true);
            _sessionHandlerList[i].transportThread(new TransportThread(i, _sessionHandlerList[i], _processMsg, this, _error));
            _sessionHandlerList[i].transportThread().channelHandler().userSpec(_sessionHandlerList[i]);
            
            _sessionHandlerList[i].transportThread().start();
        }

        /* Bind server if server mode */
        if (TransportPerfConfig.appType() == TransportPerfConfig.SERVER)
        {
            _acceptOptions.sysSendBufSize(TransportPerfConfig.sendBufSize());
            if ((_srvr = bindServer(_error)) == null)
            {
                System.out.printf("Bind failed: %s\n", _error.text());
                System.exit(-1);
            }
        }
                    
        _cpuUsageStats.clear();
        _memUsageStats.clear();
        _totalLatencyStats.clear();     
    }

    /*
     * Main loop for upajTransportPerf application.
     */
    private void run(String[] args)
    {
        // This loop will block in selector for up to 1 second and wait for accept
        // If any channel is trying to connect during this time it will be accepted
        // the time tracking parameters and counters are updated at the second interval 
        // at the configured time intervals the stats will be printed
        // the loop exits when time reaches the configured end time.
        
        long nextTime = System.currentTimeMillis() + 1000L;
        int intervalSeconds = 0;
        int currentRuntimeSec = 0; 
        int writeStatsInterval = TransportPerfConfig.writeStatsInterval();
        boolean displayStats = TransportPerfConfig.displayStats();

        // this is the main loop
        while (true)
        {
            int selectRetVal = 0;
            Set<SelectionKey> keySet = null;
            long selectTime = nextTime - System.currentTimeMillis();
            try
            {               
                if (selectTime > 0)
                    selectRetVal = _selector.select(selectTime);
                else
                    selectRetVal = _selector.selectNow();

                if (selectRetVal > 0)
                {
                    keySet = _selector.selectedKeys();
                }
            }
            catch (IOException e1)
            {
                System.out.printf("select failed\n");
                System.exit(-1);
            }

            if (selectRetVal != 0)
            {
                Iterator<SelectionKey> iter = keySet.iterator();
                while (iter.hasNext())
                {
                    SelectionKey key = iter.next();
                    iter.remove();
                    if (!key.isValid())
                        continue;
                    if (key.isAcceptable())
                    {
                        Server upajServer = (Server)key.attachment();
                        Channel clientChannel = upajServer.accept(_acceptOptions, _error);
                        if (clientChannel == null)
                        {
                            System.err.printf("UPA Server Accept failed: <%s>\n", _error.text());
                        }
                        else
                        {
                            System.err.printf("Server accepting new channel '%s'.\n\n", clientChannel.selectableChannel());
                            sendToLeastLoadedThread(clientChannel);
                        }
                    }
                }
            }

            if (System.currentTimeMillis() >= nextTime)
            {
                nextTime += 1000;
                ++intervalSeconds;
                ++currentRuntimeSec;

                // Check if it's time to print stats
                if (intervalSeconds >= writeStatsInterval)
                {
                    collectStats(true, displayStats, currentRuntimeSec, writeStatsInterval);
                    intervalSeconds = 0;
                }
            }           
            /* Handle run-time */
            handleRuntime(currentRuntimeSec);
        }
    }

    /* Collect statistics. */

    private void collectStats(boolean writeStats, boolean displayStats, int currentRuntimeSec, int timePassedSec)
    {
        long intervalMsgSentCount = 0, intervalBytesSent = 0,
            intervalMsgReceivedCount = 0, intervalBytesReceived = 0,
            intervalOutOfBuffersCount = 0;
        double currentProcessCpuLoad = ResourceUsageStats.currentProcessCpuLoad();
        double currentMemoryUsage = ResourceUsageStats.currentMemoryUsage();

        if (timePassedSec > 0)
        {
            _cpuUsageStats.update(currentProcessCpuLoad);
            _memUsageStats.update(currentMemoryUsage);
        }

        for(int i = 0; i < _sessionHandlerCount; ++i)
        {
            TimeRecordQueue latencyRecords = _sessionHandlerList[i].latencyRecords();
            _intervalLatencyStats.clear();
            
            while (!latencyRecords.records().isEmpty())
            {
                TimeRecord record = latencyRecords.records().poll();
                double latency = (double)(record.endTime() - record.startTime())/(double)record.ticks();
                if (record.startTime() > record.endTime()) 
                {   // if the start time is after the end time, then there is probably an issue with timing on the machine
                    if (!negativeLatencyFound) 
                    {
                        System.out.printf("Warning: negative latency calculated. The clocks do not appear to be synchronized. (start=%d, end=%d)\n", record.startTime(), record.endTime());
                        _summaryFileWriter.printf("Warning: negative latency calculated. The clocks do not appear to be synchronized. (start=%d, end=%d)\n", record.startTime(), record.endTime());
                    }

                    negativeLatencyFound = true;
                }
                _intervalLatencyStats.update(latency);
                _sessionHandlerList[i].transportThread().latencyStats().update(latency);
                _totalLatencyStats.update(latency);

                if (TransportThreadConfig.logLatencyToFile())
                {
                    _sessionHandlerList[i].transportThread().latencyLogFile().printf("%d, %d, %d\n", record.startTime(), record.endTime(), record.endTime() - record.startTime());
                }
            }

            if (TransportThreadConfig.logLatencyToFile())
            {
                _sessionHandlerList[i].transportThread().latencyLogFile().flush();
            }

            intervalMsgSentCount = _sessionHandlerList[i].transportThread().msgsSent().getChange();
            intervalBytesSent = _sessionHandlerList[i].transportThread().bytesSent().getChange();
            intervalMsgReceivedCount = _sessionHandlerList[i].transportThread().msgsReceived().getChange();
            intervalBytesReceived = _sessionHandlerList[i].transportThread().bytesReceived().getChange();
            intervalOutOfBuffersCount = _sessionHandlerList[i].transportThread().outOfBuffersCount().getChange();

            _totalMsgSentCount += intervalMsgSentCount;
            _totalBytesSent += intervalBytesSent;
            _totalMsgReceivedCount += intervalMsgReceivedCount;
            _totalBytesReceived += intervalBytesReceived;

            if (writeStats)
            {
                TransportThread thread = _sessionHandlerList[i].transportThread();

                printCurrentTimeUTC(thread.statsFile());
                thread.statsFile().printf(", %d, %d, %d, %d, %d, %.3f, %.3f, %.3f, %.3f, %.2f, %.2f\n", 
                        intervalMsgSentCount,
                        intervalBytesSent,
                        intervalMsgReceivedCount,
                        intervalBytesReceived,
                        _intervalLatencyStats.count(),
                        _intervalLatencyStats.average(),
                        Math.sqrt(_intervalLatencyStats.variance()),
                        _intervalLatencyStats.count() > 0 ? _intervalLatencyStats.maxValue() : 0.0,
                        _intervalLatencyStats.count() > 0 ? _intervalLatencyStats.minValue() : 0.0,
                        currentProcessCpuLoad,
                        currentMemoryUsage);
                thread.statsFile().flush();
            }

            if (displayStats)
            {
                if (TransportPerfConfig.threadCount() == 1)
                    System.out.printf("%03d:\n", currentRuntimeSec);
                else
                    System.out.printf("%03d: Thread %d:\n", currentRuntimeSec, i + 1);

                System.out.printf("  Sent: MsgRate: %8.0f, DataRate:%8.3fMBps\n",
                        (double)intervalMsgSentCount / (double)timePassedSec,
                        (double)intervalBytesSent / (double)(1024*1024) / (double)timePassedSec);

                System.out.printf("  Recv: MsgRate: %8.0f, DataRate:%8.3fMBps\n",
                        (double)intervalMsgReceivedCount / (double)timePassedSec,
                        (double)intervalBytesReceived / (double)(1024*1024) / (double)timePassedSec);

                if (intervalOutOfBuffersCount > 0)
                {
                    System.out.printf("  %d messages not sent due to lack of output buffers.\n", intervalOutOfBuffersCount);
                }

                if (_intervalLatencyStats.count() > 0)
                {
                    _intervalLatencyStats.print("  Latency(usec)", "Msgs", true);
                }

                if(TransportPerfConfig.takeMCastStats())
                {
                    long intervalMcastPacketsSent = 0, intervalMcastPacketsReceived = 0, intervalMcastRetransSent = 0, intervalMcastRetransReceived = 0;
                    SessionHandler handler = _sessionHandlerList[i];

                    for(ClientChannelInfo clientChannelInfo : handler.transportThread().channelHandler().activeChannelList())
                    {
                        if (clientChannelInfo.channel.state() != ChannelState.ACTIVE)
                            continue;

                        ChannelInfo chnlInfo = TransportFactory.createChannelInfo();
                        if (clientChannelInfo.channel.info(chnlInfo, _error) != TransportReturnCodes.SUCCESS)
                        {
                            System.out.printf("Channel.info() failed. errorId = %d (%s)\n", _error.errorId(), _error.text());
                            continue;
                        }
                        intervalMcastPacketsSent = chnlInfo.multicastStats().mcastSent()
                        		- handler.prevMCastStats().mcastSent();

                        intervalMcastPacketsReceived = chnlInfo.multicastStats().mcastRcvd() 
                        		- handler.prevMCastStats().mcastRcvd();

                        intervalMcastRetransSent = chnlInfo.multicastStats().retransPktsSent() 
                            - handler.prevMCastStats().retransPktsSent();

                        intervalMcastRetransReceived = chnlInfo.multicastStats().retransPktsRcvd() 
                            - handler.prevMCastStats().retransPktsRcvd();

                        handler.prevMCastStats(chnlInfo.multicastStats());
                    }

                    System.out.printf("  Multicast: Pkts Sent: %d, Pkts Received: %d, : Retrans sent: %d, Retrans received: %d\n",
                            intervalMcastPacketsSent, intervalMcastPacketsReceived,  intervalMcastRetransSent, intervalMcastRetransReceived);
                }

                System.out.printf("  CPU: %6.2f%% Mem: %8.2fMB\n", currentProcessCpuLoad, currentMemoryUsage);
            }
        }
    }

    private void handleRuntime(int currentRunTime)
    {
        if (currentRunTime >= TransportPerfConfig.runTime())
        {
            System.out.printf("\nRun time of %d seconds has expired.\n\n", TransportPerfConfig.runTime());
            _runTimeExpired = true;
        }

        if (_runTimeExpired == true || _shutdownApp == true)
        {
            for (int i = 0; i < _sessionHandlerCount; ++i)
            {
                _sessionHandlerList[i].transportThread().shutdown(true);
                
                int timeout = 0;
                
                // wait for consumer thread cleanup or timeout
                while (!_sessionHandlerList[i].transportThread().shutdownAck() && timeout < 1)
                {               
                    try
                    {
                        Thread.sleep(1000);
                        timeout++;
                    }
                    catch (InterruptedException e)
                    {
                        System.out.printf("Thread.sleep(1000) failed\n");
                        System.exit(-1);
                    }
                }
            }
        }       
        
        if (_runTimeExpired == true || _shutdownApp == true)
        {
            cleanUpAndExit();   
        }       
    }

    private void cleanUpAndExit()
    {
        System.out.println("Shutting down.\n");

        /* if server, close it */
        if (TransportPerfConfig.appType() == TransportPerfConfig.SERVER)
        {
            try
            {
                _selector.close();
            }
            catch (IOException e)
            {
                System.err.println("Error: Selector failure");
            }
            _srvr.close(_error);
        }
        
        Transport.uninitialize();

        collectStats(false, false, 0, 0);
        printSummaryStats(_stdOutWriter);
        _stdOutWriter.flush();
        printSummaryStats(_summaryFileWriter);
        _summaryFileWriter.close();

        System.exit(0);
    }

    private void printSummaryStats(PrintWriter fileWriter)
    {
        long earliestConnectTime, latestDisconnectTime;
        double connectedTime;

        earliestConnectTime = 0;
        latestDisconnectTime = 0;
        for(int i = 0; i < TransportPerfConfig.threadCount(); ++i)
        {
            if (earliestConnectTime == 0 ||
                _sessionHandlerList[i].transportThread().connectTime() < earliestConnectTime)
            {
                earliestConnectTime = _sessionHandlerList[i].transportThread().connectTime();
            }

            if (latestDisconnectTime == 0 ||
                _sessionHandlerList[i].transportThread().disconnectTime() > latestDisconnectTime)
            {
                latestDisconnectTime = _sessionHandlerList[i].transportThread().disconnectTime();
            }   
        }

        connectedTime = ((double)latestDisconnectTime - (double)earliestConnectTime)/1000000000.0;
        
        if (TransportPerfConfig.threadCount() > 1)
        {
            for(int i = 0; i < TransportPerfConfig.threadCount(); ++i)
            {
                TransportThread thread = _sessionHandlerList[i].transportThread();

                double threadConnectedTime = ((double)_sessionHandlerList[i].transportThread().disconnectTime() 
                        - (double)_sessionHandlerList[i].transportThread().connectTime()) / 1000000000.0;

                fileWriter.printf("--- THREAD %d SUMMARY ---\n\n", i + 1);

                fileWriter.printf("Statistics: \n");

                if (thread.latencyStats().count() > 0)
                {
                    fileWriter.printf("  Latency avg (usec): %.3f\n", thread.latencyStats().average());
                    fileWriter.printf("  Latency std dev (usec): %.3f\n", Math.sqrt(thread.latencyStats().variance()));
                    fileWriter.printf("  Latency max (usec): %.3f\n", thread.latencyStats().maxValue());
                    fileWriter.printf("  Latency min (usec): %.3f\n", thread.latencyStats().minValue());
                }
                else
                    fileWriter.printf("  No latency information was received.\n\n");

                fileWriter.printf("  Sampling duration(sec): %.2f\n", threadConnectedTime);
                fileWriter.printf("  Msgs Sent: %d\n", thread.msgsSent().getTotal());
                fileWriter.printf("  Msgs Received: %d\n", thread.msgsReceived().getTotal());
                fileWriter.printf("  Data Sent (MB): %.2f\n", (double)thread.bytesSent().getTotal()/1048576.0);
                fileWriter.printf("  Data Received (MB): %.2f\n", (double)thread.bytesReceived().getTotal()/1048576.0);
                fileWriter.printf("  Avg. Msg Sent Rate: %.0f\n", threadConnectedTime > 0 ? (double)thread.msgsSent().getTotal()/threadConnectedTime : 0);
                fileWriter.printf("  Avg. Msg Recv Rate: %.0f\n", threadConnectedTime > 0 ? (double)thread.msgsReceived().getTotal()/threadConnectedTime : 0);
                fileWriter.printf("  Avg. Data Sent Rate (MB): %.2f\n", threadConnectedTime > 0 ? (double)thread.bytesSent().getTotal()/1048576.0/threadConnectedTime : 0);
                fileWriter.printf("  Avg. Data Recv Rate (MB): %.2f\n\n", threadConnectedTime > 0 ? (double)thread.bytesReceived().getTotal()/1048576.0/threadConnectedTime : 0);
            }
        }

        fileWriter.printf("--- OVERALL SUMMARY ---\n\n");

        fileWriter.printf("Statistics: \n");

        if (_totalLatencyStats.count() > 0)
        {
            fileWriter.printf("  Latency avg (usec): %.3f\n", _totalLatencyStats.average());
            fileWriter.printf("  Latency std dev (usec): %.3f\n", Math.sqrt(_totalLatencyStats.variance()));
            fileWriter.printf("  Latency max (usec): %.3f\n", _totalLatencyStats.maxValue());
            fileWriter.printf("  Latency min (usec): %.3f\n", _totalLatencyStats.minValue());
        }
        else
            fileWriter.printf("  No latency information was received.\n\n");

        fileWriter.printf("  Sampling duration(sec): %.2f\n", connectedTime);
        fileWriter.printf("  Msgs Sent: %d\n", _totalMsgSentCount);
        fileWriter.printf("  Msgs Received: %d\n", _totalMsgReceivedCount);
        fileWriter.printf("  Data Sent (MB): %.2f\n", (double)_totalBytesSent/1048576.0);
        fileWriter.printf("  Data Received (MB): %.2f\n", (double)_totalBytesReceived/1048576.0);
        fileWriter.printf("  Avg. Msg Sent Rate: %.0f\n", connectedTime > 0 ? (double)_totalMsgSentCount/connectedTime : 0);
        fileWriter.printf("  Avg. Msg Recv Rate: %.0f\n", connectedTime > 0 ? (double)_totalMsgReceivedCount/connectedTime : 0);
        fileWriter.printf("  Avg. Data Sent Rate (MB): %.2f\n", connectedTime > 0 ? (double)_totalBytesSent/1048576.0/connectedTime : 0);
        fileWriter.printf("  Avg. Data Recv Rate (MB): %.2f\n", connectedTime > 0 ? (double)_totalBytesReceived/1048576.0/connectedTime : 0);

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
        else
            System.out.printf("No CPU/Mem statistics taken.\n\n");

        fileWriter.printf("  Process ID: %d\n", Thread.currentThread().getId());
    }

    private void sendToLeastLoadedThread(Channel chnl)
    {
        SessionHandler connHandler = null;
        int minProviderConnCount, connHandlerIndex = 0;

        minProviderConnCount = 0x7fffffff;
        for(int i = 0; i < _sessionHandlerCount; ++i)
        {
            SessionHandler tmpHandler = _sessionHandlerList[i];
            tmpHandler.handlerLock().lock();
            if (tmpHandler.openChannelsCount() < minProviderConnCount)
            {
                minProviderConnCount = tmpHandler.openChannelsCount();
                connHandler = tmpHandler;
                connHandlerIndex = i;
            }
            tmpHandler.handlerLock().unlock();
        }

        connHandler.handlerLock().lock();
        connHandler.openChannelsCount(connHandler.openChannelsCount() + 1);
        connHandler.newChannelsList().add(chnl);
        connHandler.handlerLock().unlock();

        System.out.println("Server: " + _srvr.selectableChannel());
        System.out.printf("New Channel %s passed connection to handler %d\n\n", chnl.selectableChannel(), connHandlerIndex);
    }

    private Server bindServer(Error error)
    {
        Server srvr;
        BindOptions sopts = TransportFactory.createBindOptions();
        
        sopts.guaranteedOutputBuffers(TransportPerfConfig.guaranteedOutputBuffers());

        if (TransportPerfConfig.interfaceName() != null)
            sopts.interfaceName(TransportPerfConfig.interfaceName());

        sopts.majorVersion(0);
        sopts.minorVersion(0);
        sopts.protocolType(TransportThread.TEST_PROTOCOL_TYPE);
        sopts.tcpOpts().tcpNoDelay(TransportPerfConfig.tcpNoDelay());
        sopts.connectionType(TransportPerfConfig.connectionType());
        sopts.maxFragmentSize(TransportPerfConfig.maxFragmentSize());
        sopts.compressionType(TransportPerfConfig.compressionType());
        sopts.compressionLevel(TransportPerfConfig.compressionLevel());
        sopts.serviceName(TransportPerfConfig.portNo());
        sopts.sysRecvBufSize(TransportPerfConfig.recvBufSize());

        if ((srvr = Transport.bind(sopts, error)) == null)
            return null;

        System.out.println("\nServer bound on port " + srvr.portNumber());
        try
        {
            srvr.selectableChannel().register(_selector, SelectionKey.OP_ACCEPT, srvr);
        }
        catch (Exception exception)
        {
            System.err.println("Error: Selector failure");
            System.exit(-1);
        }

        return srvr;
    }

    /* Prints the current time, in Coordinated Universal Time. */
    private void printCurrentTimeUTC(PrintWriter fileWriter)
    {
        _dateTime.gmtTime();

        fileWriter.printf("%d-%02d-%02d %02d:%02d:%02d",
                _dateTime.year(), _dateTime.month(), _dateTime.day(),
                _dateTime.hour(), _dateTime.minute(), _dateTime.second());
    }
        
    public void attachShutDownHook(){
          Runtime.getRuntime().addShutdownHook(new Thread() {
         
              public void run() {
                  shutdown();
              }
          });
    }   
    
    public static void main(String[] args)
    {
        upajTransportPerf transportperf = new upajTransportPerf();
        transportperf.attachShutDownHook();
        transportperf.initialize(args);
        transportperf.run(args);
        System.exit(0);
    }
}
