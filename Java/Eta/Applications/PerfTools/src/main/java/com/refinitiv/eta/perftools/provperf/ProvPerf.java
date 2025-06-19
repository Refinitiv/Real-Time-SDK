/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.provperf;

import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.perftools.common.Provider;
import com.refinitiv.eta.perftools.common.ProviderPerfConfig;
import com.refinitiv.eta.perftools.common.ProviderType;
import com.refinitiv.eta.perftools.common.XmlMsgData;
import com.refinitiv.eta.transport.AcceptOptions;
import com.refinitiv.eta.transport.BindOptions;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InitArgs;
import com.refinitiv.eta.transport.Server;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;

/**
 * The ProvPerf application. Implements an interactive provider, which
 * allows the requesting of items, and responds to them with images and bursts
 * of updates.
 * <p>
 * The purpose of this application is to measure performance of the ETA
 * transport, encoders and decoders, in providing Level I Market Price content
 * to the LSEG Real-Time Distribution System Advanced Data Hub (ADH).
 * <p>
 * <em>Summary</em>
 * <p>
 * The provider creates two categories of threads:
 * <ul>
 * <li>A main thread, which collects and records statistical information.
 * <li>Provider threads, each of which connect to an ADH and provide market
 * data.
 * </ul>
 * <p>
 * The provider may be configured to provide updates at various rates. To
 * measure latency, a timestamp is randomly placed in each burst of updates. The
 * consumer then decodes the timestamp from the update to determine the
 * end-to-end latency.
 * <p>
 * This application also measures memory and CPU usage. Java 7 (Oracle JDK)
 * introduced OperatingSystemMXBean which is a platform-specific management
 * interface for the operating system on which the Java virtual machine is
 * running. The getCommittedVirtualMemorySize() method is used for memory usage
 * and the getProcessCpuLoad() method is used for CPU usage.
 * <p>
 * For more detailed information on the performance measurement applications,
 * see the ETAJ Open Source Performance Tools Guide
 * (Docs/ETAJ_PerfToolsGuide.pdf).
 * <p>
 * This application uses XML Pull Parser (XPP), an open source XML parser
 * library.
 * <p>
 * <em>Setup Environment</em>
 * <p>
 * The following configuration files are required:
 * <ul>
 * <li>RDMFieldDictionary and enumtype.def, located in the etc directory.
 * <li>350k.xml, located in PerfTools
 * <li>MsgData.xml, located in PerfTools
 * </ul>
 * <p>
 * <em>Running the application:</em>
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runETAPerfProvider -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runETAPerfProvider -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below.
 * </p>
 * <i>-help</i> displays all command line arguments, with a brief description of each one
 */
public class ProvPerf
{
    // The provider used by this application. Provides the logic that
    // connections use with ProvPerf and NIProvPerf for maintaining
    // channels and open items for a connection, and manages the sending of
    // refreshes and updates.
    private Provider _provider;                
    
    private XmlMsgData _xmlMsgData;             // message data information from XML file

    private Error _error;                        // error information
    private Selector _selector;                  // selector
    private BindOptions _bindOptions;            // server bind options
    private AcceptOptions _acceptOptions;        // server accept options
    private Server _server;                      // eta server
    private InitArgs _initArgs;                  // init arguments

    private volatile boolean _shutdownApp;       // Indicates whether or not
                                                 // application should be
                                                 // shutdown

    private AtomicBoolean _stop = new AtomicBoolean(true);
    private volatile boolean _exitApp;

    private CountDownLatch _loopExited = new CountDownLatch(1);
    private CountDownLatch _stopped = new CountDownLatch(1);

    public ProvPerf()
    {
        _provider = new Provider();
        _error = TransportFactory.createError();
        _bindOptions = TransportFactory.createBindOptions();
        _initArgs = TransportFactory.createInitArgs();
        _acceptOptions = TransportFactory.createAcceptOptions();
        _xmlMsgData = new XmlMsgData();
    }

    /*
     * Initializes etaj performance application with  
     */
    private void init(String[] args)
    {
        ProviderPerfConfig.init(args);
        //init binding options first to be able print effective values
        initBindOptions();
        _acceptOptions.sysSendBufSize(ProviderPerfConfig.sendBufSize());
        System.out.println(ProviderPerfConfig.convertToString(_bindOptions));

        // parse message data XML file
        if (_xmlMsgData.parseFile(ProviderPerfConfig.msgFilename()) == CodecReturnCodes.FAILURE)
        {
            System.out.printf("Failed to load message data from file '%s'.\n", ProviderPerfConfig.msgFilename());
            System.exit(-1);
        }

        // The application will exit if error happens during initialization
        _provider.init(ProviderType.PROVIDER_INTERACTIVE, _xmlMsgData, ProviderPerfConfig.summaryFilename());

        _provider.startThreads();

        _initArgs.clear();
        if (!ProviderPerfConfig.useReactor()) // use ETA Channel for sending and receiving
        {
            _initArgs.globalLocking(ProviderPerfConfig.threadCount() > 1 ? true : false);
        }
        else // use ETA VA Reactor
        {
            _initArgs.globalLocking(true);
        }
        if (Transport.initialize(_initArgs, _error) != TransportReturnCodes.SUCCESS)
        {
            System.err.println("Error: Transport failed to initialize: " + _error.text());
            System.exit(-1);
        }

        try
        {
            _selector = Selector.open();
        }
        catch (Exception exception)
        {
            System.err.println("Error: Selector open failure");
            System.exit(-1);
        }

        _server = Transport.bind(_bindOptions, _error);
        if (_server == null)
        {
            System.err.println("Error: Transport bind failure: " + _error.text());
            System.exit(-1);
        }
        System.out.println("\nServer bound on port " + _server.portNumber());

        try
        {
            _server.selectableChannel().register(_selector, SelectionKey.OP_ACCEPT, _server);
        }
        catch (Exception e)
        {
            System.err.println("Error: Selector failure");
            System.exit(-1);
        }
    }

	private void initBindOptions() {
		_bindOptions.guaranteedOutputBuffers(ProviderPerfConfig.guaranteedOutputBuffers());
        if (ProviderPerfConfig.maxOutputBuffers() > 0)
            _bindOptions.maxOutputBuffers(ProviderPerfConfig.maxOutputBuffers());
        _bindOptions.serviceName(ProviderPerfConfig.portNo());
        if (ProviderPerfConfig.interfaceName() != null)
            _bindOptions.interfaceName(ProviderPerfConfig.interfaceName());
        
        _bindOptions.connectionType(ProviderPerfConfig.connType());
        if(_bindOptions.connectionType() == ConnectionTypes.ENCRYPTED)
        {
        	_bindOptions.encryptionOptions().keystoreFile(ProviderPerfConfig.keyfile());
        	_bindOptions.encryptionOptions().keystorePasswd(ProviderPerfConfig.keypasswd());
            String securityProvider = ProviderPerfConfig.securityProvider();
            if(securityProvider != null && !securityProvider.isEmpty())
                _bindOptions.encryptionOptions().securityProvider(securityProvider);
        }

        _bindOptions.majorVersion(Codec.majorVersion());
        _bindOptions.minorVersion(Codec.minorVersion());
        _bindOptions.protocolType(Codec.protocolType());
        _bindOptions.sysRecvBufSize(ProviderPerfConfig.recvBufSize());
        _bindOptions.maxFragmentSize(ProviderPerfConfig.maxFragmentSize());
        _bindOptions.tcpOpts().tcpNoDelay(ProviderPerfConfig.tcpNoDelay());
        _bindOptions.wSocketOpts().protocols(ProviderPerfConfig.protocolList());
	}

    /*
     * Main loop for provider perf application.
     */
    private void run()
    {
    	// This loop will block in selector for up to 1 second and wait for accept
    	// If any channel is trying to connect during this time it will be accepted
    	// the time tracking parameters and counters are updated at the second interval 
    	// at the configured time intervals the stats will be printed
    	// the loop exits when time reaches the configured end time.
    	
        long nextTime = System.currentTimeMillis() + 1000L;
        int intervalSeconds = 0;
        int currentRuntimeSec = 0; 
        int writeStatsInterval = ProviderPerfConfig.writeStatsInterval();
        int runTime = ProviderPerfConfig.runTime();
        boolean displayStats = ProviderPerfConfig.displayStats();
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();

        // this is the main loop
        while (!_shutdownApp && !_exitApp)
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
        				Server server = (Server)key.attachment();
                        if (!ProviderPerfConfig.useReactor()) // use ETA Channel
                        {
        				Channel clientChannel = server.accept(_acceptOptions, _error);
        				if (clientChannel == null)
        				{
        					System.err.printf("ETA Server Accept failed: <%s>\n", _error.text());
        				}
        				else
        				{
        					System.err.printf("Server accepting new channel '%s'.\n\n", clientChannel.selectableChannel());
        					sendToLeastLoadedThread(clientChannel);
        				}
        			}
        				else // use ETA VA Reactor
        				{
                            if (acceptReactorConnection(server, errorInfo) != ReactorReturnCodes.SUCCESS)
                            {
                                System.err.printf("acceptReactorConnection: failed <%s>\n", errorInfo.error().text());
                            }
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
        			_provider.collectStats(true, displayStats, currentRuntimeSec, writeStatsInterval);
        			intervalSeconds = 0;
        		}
        	}

        	// Handle run-time
        	if (currentRuntimeSec >= runTime)
        	{
        		System.out.printf("\nRun time of %d seconds has expired.\n\n", runTime);
        		break;
        	}

        }

        _loopExited.countDown();
        if (_stop.getAndSet(false)) {
            stop();
            _stopped.countDown();
        }
    }

    private void stop() {
        try {
            stopProviderThreads();
            // only print summary on normal exit
            if (!_shutdownApp)
            {
                _provider.printFinalStats();
            }
            cleanup();
        } finally {
            _stopped.countDown();
        }
    }

    // Stop all provider threads.
    private void stopProviderThreads()
    {
        for (int i = 0; i < ProviderPerfConfig.threadCount(); i++)
        {
            _provider.providerThreadList()[i].shutdown(true);
        }

        for (int i = 0; i < ProviderPerfConfig.threadCount(); i++)
        {
            int shutdownCount = 0;
            // wait for provider thread cleanup or timeout
            while (!_provider.providerThreadList()[i].shutdownAck() && shutdownCount < 3)
            {
                try
                {
                    Thread.sleep(1000);
                    shutdownCount++;
                }
                catch (InterruptedException e)
                {
                    System.err.printf("Thread.sleep(1000) failed\n");
                }
            }
        }

        System.out.println("Shutting down.\n");
    }

    /*
     * Adds new client channel to provider thread's new client session list.
     */
    private void sendToLeastLoadedThread(Channel channel)
    {
    	IProviderThread provThread = null;

        int minProvConnCount = 0x7fffffff;

        for (int i = 0; i < ProviderPerfConfig.threadCount(); ++i)
        {
        	IProviderThread tmpProvThread = (IProviderThread) _provider.providerThreadList()[i];
        	tmpProvThread.handlerLock().lock();
            int connCount = tmpProvThread.connectionCount();
            if (connCount < minProvConnCount)
            {
                minProvConnCount = connCount;
                provThread = tmpProvThread;
            }
            tmpProvThread.handlerLock().unlock();
        }

        provThread.handlerLock().lock();
        provThread.acceptNewChannel(channel);
        provThread.handlerLock().unlock();
    }

    private int acceptReactorConnection(Server server, ReactorErrorInfo errorInfo)
    {   
        IProviderThread provThread = null;

        int minProvConnCount = 0x7fffffff;

        // find least loaded thread
        for (int i = 0; i < ProviderPerfConfig.threadCount(); ++i)
        {
            IProviderThread tmpProvThread = (IProviderThread) _provider.providerThreadList()[i];
            int connCount = tmpProvThread.connectionCount();
            if (connCount < minProvConnCount)
            {
                minProvConnCount = connCount;
                provThread = tmpProvThread;
            }
        }

        // accept new reactor channel
        return provThread.acceptNewReactorChannel(server, errorInfo);
    }

    /*
     * Clean up provider thread app.
     */
    private void cleanup()
    {
        _provider.cleanup();

        _server.close(_error);

        Transport.uninitialize();
    }

    private void registerShutdownHook() {
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            try {
                if (_stop.getAndSet(false)) {
                    _exitApp = true;
                    _loopExited.await(1000, TimeUnit.MILLISECONDS);
                    stop();
                } else {
                    _stopped.await(ProviderPerfConfig.threadCount() * 5000, TimeUnit.MILLISECONDS);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }));
    }

    public static void main(String[] args)
    {
        ProvPerf provperf = new ProvPerf();
        provperf.init(args);
        provperf.registerShutdownHook();
        provperf.run();
        System.exit(0);
    }
}

