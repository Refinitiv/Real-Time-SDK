package com.refinitiv.eta.perftools.niprovperf;

import com.refinitiv.eta.perftools.common.NIProvPerfConfig;
import com.refinitiv.eta.perftools.common.PerfToolsReturnCodes;
import com.refinitiv.eta.perftools.common.Provider;
import com.refinitiv.eta.perftools.common.ProviderPerfConfig;
import com.refinitiv.eta.perftools.common.ProviderType;
import com.refinitiv.eta.perftools.common.XmlMsgData;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InitArgs;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;

/**
 * <p>
 * The main NIProvPerf application. Implements a Non-Interactive Provider.
 * It connects to an ADH, and provides images and update bursts for a given
 * number of items.
 * <p>
 * The purpose of this application is to measure performance of the ETA transport, 
 * encoders and decoders, in providing Level I Market Price content to the
 * Refinitiv Real-Time Distribution System Advanced Data Hub (ADH).
 * <p>
 * <em>Summary</em>
 * <p>
 * The provider creates two categories of threads:
 * <ul>
 * <li>A main thread, which collects and records statistical information.
 * <li>Multiple (configured number) provider threads, each of which connects to an 
 * ADH and provides market data.
 * </ul>
 * <p>
 * The provider may be configured to provide updates at various rates. To measure
 * latency, a timestamp is randomly placed in each burst of updates. The consumer
 * then decodes the timestamp from the update to determine the end-to-end latency.
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
 * <li>MsgData.xml, located in PerfTools
 * </ul>
 * <p>
 * <em>Running the application:</em> 
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runETAPerfNIProvider -PcommandLineArgs="arguments"<br>
 * Windows: gradlew.bat runETAPerfNIProvider -PcommandLineArgs="arguments"<br>
 * <br>
 * Arguments are listed below.
 * </p>
 * <i>-help</i> displays all command line arguments, with a brief description of each one
 */
public class NIProvPerf
{	
    private XmlMsgData _xmlMsgData; /* message data information from XML file */

	/* indicates whether or not application should be shutdown */
	private volatile boolean _shutdownApp = false; 
	
	/* The provider used by this application. Handles the statistic
	 * for all threads used by this provider. */
    private Provider _provider;
    
    private Error _error; /* error information */
    private InitArgs _initArgs; /* arguments for initializing transport */
	
	{
        _provider = new Provider();		
        _error = TransportFactory.createError();
        _initArgs = TransportFactory.createInitArgs();
		_xmlMsgData = new XmlMsgData();
	}

	/** Run NIProvPerf */
	public void run()
	{
		long intervalSeconds = 0, currentRuntimeSec = 0;
		//Initialize runtime timer
		long niProvRuntime = System.nanoTime() + (NIProvPerfConfig.runTime() * 1000000000L);

		
		/* this is the main loop */
		while(!_shutdownApp)
		{
			try
			{
				Thread.sleep(1000);
			}
			catch (InterruptedException e)
			{
				System.out.printf("Thread.sleep(1000) failed\n");
				System.exit(-1);
			}

			++currentRuntimeSec;
			++intervalSeconds;

			/* collect statistics at write statistics interval */
			if (intervalSeconds == NIProvPerfConfig.writeStatsInterval())
			{
				_provider.collectStats(true, NIProvPerfConfig.displayStats(), currentRuntimeSec, NIProvPerfConfig.writeStatsInterval());
				intervalSeconds = 0;
			}
			
			/* Handle runtime. */
			if (System.nanoTime() >= niProvRuntime)
			{
				System.out.printf("\nRun time of %d seconds has expired.\n\n", NIProvPerfConfig.runTime());
				break;
			}
		}
		
		stopProviderThreads();
		
		if(!_shutdownApp)
		{
			//collect and print summary statistics
			_provider.collectStats(false, false, 0, 0);
			_provider.printFinalStats();
		}

		cleanUpAndExit();
	}
	
	/* Initializes NIProvPerf application. */
    private void initialize(String[] args)
    {
		/* Read in configuration and echo it. */
		NIProvPerfConfig.init(args);
		System.out.println(NIProvPerfConfig.convertToString());

        //parse message data XML file
	    if (_xmlMsgData.parseFile(NIProvPerfConfig.msgFilename()) == PerfToolsReturnCodes.FAILURE)
	    {
	      	System.out.printf("Failed to load message data from file '%s'.\n", NIProvPerfConfig.msgFilename());
			System.exit(-1);
		}
	    
	    // The application will exit if error happens during initialization
		_provider.init(ProviderType.PROVIDER_NONINTERACTIVE, _xmlMsgData, NIProvPerfConfig.summaryFilename());

		//Initialize ETA Transport
		if (!NIProvPerfConfig.useReactor())
		{
            _initArgs.clear();
            _initArgs.globalLocking(NIProvPerfConfig.threadCount() > 1 ? true : false);
            if (Transport.initialize(_initArgs, _error) != TransportReturnCodes.SUCCESS)
    		{
                System.err.println("Error: Transport failed to initialize: " + _error.text());
    			System.exit(-1);
    		}
		}

		_provider.startThreads();
	}

	//Stop all provider threads.
    private void stopProviderThreads()
    {
        for(int i = 0; i < ProviderPerfConfig.threadCount(); i++)
        {
            _provider.providerThreadList()[i].shutdown(true);
        }

        for(int i = 0; i < ProviderPerfConfig.threadCount(); i++)
        {
            
            // wait for provider thread cleanup or timeout
            while (!_provider.providerThreadList()[i].shutdownAck())
            {
                try
                {
                    Thread.sleep(1000);
                }
                catch (InterruptedException e)
                {
                    System.err.printf("Thread.sleep(1000) failed\n");
                    System.exit(-1);
                }
            }
        }

        System.out.println("\nShutting down.\n\n");
    }

    /* Cleanup and exit NIProvPerf application. */
	private void cleanUpAndExit()
	{
		//cleanup provider
		_provider.cleanup();

		//uninitialize ETA transport and exit
		Transport.uninitialize();
		System.out.printf("Exiting.\n");
	}

	public static void main(String[] args)
	{
		NIProvPerf niprovperf = new NIProvPerf();
		niprovperf.initialize(args);
		niprovperf.run();
		System.exit(0);
	}
}

