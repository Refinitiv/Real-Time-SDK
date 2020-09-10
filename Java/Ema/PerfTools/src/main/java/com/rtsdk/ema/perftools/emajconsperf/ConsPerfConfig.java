package com.rtsdk.ema.perftools.emajconsperf;

import com.rtsdk.ema.perftools.common.CommandLine;

/** Provides configuration that is not specific to any particular handler. */
public class ConsPerfConfig
{
	private String _configString;
	private static final int DEFAULT_THREAD_COUNT = 1;
	private int _maxThreads;

	/* APPLICATION configuration */
	private int	_steadyStateTime;			/* Time application runs before exiting. */
	private int	_ticksPerSec;				/* Main loop ticks per second */
	private int	_threadCount;				/* Number of threads that handle connections. */

	private String _itemFilename;			/* File of names to use when requesting items. */
	private String _msgFilename;			/* File of data to use for message payloads. */

	private boolean	_logLatencyToFile;		/* Whether to log update latency information to a file. */
	private String _latencyLogFilename;		/* Name of the latency log file. */
	private String _summaryFilename;		/* Name of the summary log file. */
	private String _statsFilename;			/* Name of the statistics log file. */
	private int	_writeStatsInterval;		/* Controls how often statistics are written. */
	private boolean _displayStats;			/* Controls whether stats appear on the screen. */

	private int _itemRequestsPerSec;		/* Rate at which the consumer will send out item requests. */

	private boolean _requestSnapshots;		/* Whether to request all items as snapshots. */

	private String _username;				/* Username used when logging in. */
	private String _serviceName;			/* Name of service to request items from. */
	private boolean _useServiceId;          /* set service id on each request instead of setting service name on each request. */
	private int _itemRequestCount;			/* Number of items to request. See -itemCount. */
	private int _commonItemCount;			/* Number of items common to all connections, if using multiple connections. */
	private int _postsPerSec;				/* Number of posts to send per second. */
	private int _latencyPostsPerSec;		/* Number of latency posts to send per second. */
	private int _genMsgsPerSec;				/* Number of generic msgs to send per second. */
	private int _latencyGenMsgsPerSec;		/* Number of latency generic msgs to send per second. */

	private int _requestsPerTick;			/* Number of requests to send per tick */
	private int _requestsPerTickRemainder;	/* The remainder of number of requests to send per tick */
	
	private boolean _primeJVM;				/* At startup, prime the JVM to optimize code by requesting a snapshot of all items before opening the streaming items. */
	private boolean _useUserDispatch;          /* Use the EMA  USER_DISPATCH model instead of the EMA API_DISPATCH  model. */
	private boolean _downcastDecoding;		/* Turn on the EMA data load downcast feature during decoding response payload. */
	
    {
        CommandLine.programName("emajConsPerf");
        CommandLine.addOption("steadyStateTime", 300, "Time consumer will run the steady-state portion of the test. Also used as a timeout during the startup-state portion");
        CommandLine.addOption("tickRate", 1000, "Ticks per second");
        CommandLine.addOption("threads", DEFAULT_THREAD_COUNT, "Number of threads that handle connections");
        CommandLine.addOption("itemFile", "350k.xml", "Name of the file to get item names from");
        CommandLine.addOption("msgFile", "MsgData.xml", "Name of the file that specifies the data content in messages");
        CommandLine.addOption("latencyFile", "", "Base name of file for logging latency");
        CommandLine.addOption("summaryFile", "ConsSummary.out", "Name of file for logging summary info");
        CommandLine.addOption("statsFile", "ConsStats", "Base name of file for logging periodic statistics");
        CommandLine.addOption("writeStatsInterval", 5, "Controls how often stats are written to the file");
        CommandLine.addOption("noDisplayStats", false, "Stop printout of stats to screen");
        CommandLine.addOption("requestRate", 13500, "Rate at which to request items");
        CommandLine.addOption("snapshot", false, "Snapshot test, request all items as non-streaming");
        CommandLine.addOption("uname", "", "Username to use in login request");
        CommandLine.addOption("serviceName", "DIRECT_FEED", "Name of service to request items from");
        CommandLine.addOption("useServiceId", false, "set service id on each request instead of setting service name on each request");
        CommandLine.addOption("itemCount", 100000, "Number of items to request");
        CommandLine.addOption("commonItemCount", 0, "Number of items common to all consumers, if using multiple connections");
        CommandLine.addOption("postingRate", 0, "Rate at which to send post messages");
        CommandLine.addOption("postingLatencyRate", 0, "Rate at which to send latency post messages");
        CommandLine.addOption("genericMsgRate", 0, "Rate at which to send generic messages");
        CommandLine.addOption("genericMsgLatencyRate", 0, "Rate at which to send latency generic messages");
        CommandLine.addOption("primeJVM", false, "At startup, prime the JVM to optimize code by requesting a snapshot of all items before opening the streaming items");
        CommandLine.addOption("useUserDispatch", false, "Use the EMA USER_DISPATCH model instead of the EMA API_DISPATCH model for sending and receiving");
        CommandLine.addOption("downcastDecoding", false, "Turn on the EMA data load downcast feature during decoding response payload");
    }
	
    /**
     *  Parses command-line arguments to fill in the application's configuration structures.
     *
     * @param args the args
     * @param maxThreads the max threads
     */
	public void init (String[] args, int maxThreads)
	{
        try
        {
        	CommandLine.parseArgs(args);
        }
        catch (IllegalArgumentException ile)
        {
        	System.err.println("Error loading command line arguments:\t");
        	System.err.println(ile.getMessage());
        	System.err.println();
        	System.err.println(CommandLine.optionHelpString());
        	System.exit(-1);
        }

    	_maxThreads = maxThreads;
    	_msgFilename = CommandLine.value("msgFile");
    	_itemFilename = CommandLine.value("itemFile");
    	_logLatencyToFile = false;
    	_latencyLogFilename = CommandLine.value("latencyFile");
    	_summaryFilename = CommandLine.value("summaryFile");
    	_statsFilename = CommandLine.value("statsFile");
    	_username = CommandLine.value("uname");
    	_serviceName = CommandLine.value("serviceName");
    	_useServiceId = CommandLine.booleanValue("useServiceId");
    	_displayStats = !CommandLine.booleanValue("noDisplayStats");
    	_requestSnapshots = CommandLine.booleanValue("snapshot");
    	_primeJVM = CommandLine.booleanValue("primeJVM");
    	_useUserDispatch = CommandLine.booleanValue("useUserDispatch");
        _downcastDecoding = CommandLine.booleanValue("downcastDecoding");
        try
        {
        	_steadyStateTime = CommandLine.intValue("steadyStateTime");
        	_ticksPerSec = CommandLine.intValue("tickRate");
        	_threadCount = CommandLine.intValue("threads");
        	_writeStatsInterval = CommandLine.intValue("writeStatsInterval");
        	_itemRequestsPerSec = CommandLine.intValue("requestRate");
        	_itemRequestCount = CommandLine.intValue("itemCount");
        	_commonItemCount = CommandLine.intValue("commonItemCount");
        	_postsPerSec = CommandLine.intValue("postingRate");
        	_latencyPostsPerSec = CommandLine.intValue("postingLatencyRate");
        	_genMsgsPerSec = CommandLine.intValue("genericMsgRate");
        	_latencyGenMsgsPerSec = CommandLine.intValue("genericMsgLatencyRate");
        }
        catch (NumberFormatException ile)
        {
        	System.err.println("Invalid argument, number expected.\t");
        	System.err.println(ile.getMessage());
        	System.err.println();
        	System.err.println(CommandLine.optionHelpString());
        	System.exit(-1);
        }
        
        if (_ticksPerSec < 1)
        {
            System.err.println("Config Error: Tick rate cannot be less than 1.");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        } 
        
		if (_itemRequestsPerSec < _ticksPerSec)
		{
			System.err.println("Config Error: Item Request Rate cannot be less than tick rate.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		if (_postsPerSec < _ticksPerSec && _postsPerSec != 0)
		{
			System.err.println("Config Error: Post Rate cannot be less than tick rate(unless it is zero).\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		if (_latencyPostsPerSec > _postsPerSec)
		{
			System.err.println("Config Error: Latency Post Rate cannot be greater than total posting rate.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		if (_latencyPostsPerSec > _ticksPerSec)
		{
			System.err.println("Config Error: Latency Post Rate cannot be greater than tick rate.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		if (_postsPerSec > 0 && _requestSnapshots)
		{
			System.err.println("Config Error: Configured to post while requesting snapshots.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		if (_genMsgsPerSec < _ticksPerSec && _genMsgsPerSec != 0)
		{
			System.err.println("Config Error: Generic Msg Rate cannot be less than tick rate(unless it is zero).\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		if (_latencyGenMsgsPerSec > _genMsgsPerSec)
		{
			System.err.println("Config Error: Latency Generic Msg Rate cannot be greater than total generic msg rate.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		if (_latencyGenMsgsPerSec > _ticksPerSec)
		{
			System.err.println("Config Error: Latency Generic Msg Rate cannot be greater than tick rate.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		if (_genMsgsPerSec > 0 && _requestSnapshots)
		{
			System.err.println("Config Error: Configured to send generic msgs while requesting snapshots.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}
		
		if (_commonItemCount > _itemRequestCount)
		{
			System.err.println("Config Error: Common item count is greater than total item count.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		if (_writeStatsInterval < 1)
		{
			System.err.println("Config error: Write Stats Interval cannot be less than 1.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}
		
		if (_threadCount > _maxThreads)
		{
			System.err.println("Config error: Thread count cannot be greater than " + _maxThreads + ".\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		_requestsPerTick = _itemRequestsPerSec / _ticksPerSec;

		_requestsPerTickRemainder = _itemRequestsPerSec % _ticksPerSec;
		
		createConfigString();
	}
    
	/* Create config string. */
	private void createConfigString()
	{
	    String useOperationModelUsageString;

	    if (_useUserDispatch)
	    {
	        useOperationModelUsageString = "USER_DISPATCH";
	    }
	    else
	    {
	    	useOperationModelUsageString = "API_DISPATCH";
	    }
	   
		_configString = "--- TEST INPUTS ---\n\n" +
				"       Steady State Time: " + _steadyStateTime + " sec\n" + 
				"                 Service: " + _serviceName + "\n" +
				"            UseServiceId: " + (_useServiceId ? "Yes" : "No") + "\n" +
				"            Thread Count: " + _threadCount + "\n" +
				"                Username: " + (_username.length() > 0 ? _username : "(use system login name)") + "\n" +
				"              Item Count: " + _itemRequestCount + "\n" +
				"       Common Item Count: " + _commonItemCount + "\n" +
				"            Request Rate: " + _itemRequestsPerSec + "\n" +
				"       Request Snapshots: " + (_requestSnapshots ? "Yes" : "No") + "\n" +
				"            Posting Rate: " + _postsPerSec + "\n" +
				"    Latency Posting Rate: " + _latencyPostsPerSec + "\n" +
				"        Generic Msg Rate: " + _genMsgsPerSec + "\n" +
				"Generic Msg Latency Rate: " + _latencyGenMsgsPerSec + "\n" +
				"               Item File: " + _itemFilename + "\n" +
				"               Data File: " + _msgFilename + "\n" +
				"            Summary File: " + _summaryFilename + "\n" +
				"              Stats File: " + _statsFilename + "\n" +
				"        Latency Log File: " + (_latencyLogFilename.length() > 0 ? _latencyLogFilename : "(none)") + "\n" +
				"               Tick Rate: " + _ticksPerSec + "\n" +
				"               Prime JVM: " + (_primeJVM ? "Yes" : "No") + "\n" +
				"        DowncastDecoding: " + (_downcastDecoding ? "True" : "False") + "\n" +
				"    OperationModel Usage: " + useOperationModelUsageString + "\n";
	}

	/* APPLICATION configuration */
    /**
	 *  Time application runs before exiting.
	 *
	 * @return the int
	 */
	public int	steadyStateTime()
	{
		return _steadyStateTime;
	}
	
	/**
	 *  Main loop ticks per second.
	 *
	 * @return the int
	 */
	public int	ticksPerSec()
	{
		return _ticksPerSec;
	}
	
	/**
	 *  Number of threads that handle connections.
	 *
	 * @return the int
	 */
	public int	threadCount()
	{
		return _threadCount;
	}

	/**
	 *  File of names to use when requesting items.
	 *
	 * @return the string
	 */
	public String itemFilename()
	{
		return _itemFilename;
	}
	
	/**
	 *  File of data to use for message payloads.
	 *
	 * @return the string
	 */
	public String msgFilename()
	{
		return _msgFilename;
	}

	/**
	 *  Whether to log update latency information to a file.
	 *
	 * @return true, if successful
	 */
	public boolean	logLatencyToFile()
	{
		_logLatencyToFile = (_latencyLogFilename.length() > 0 ? true : false);
		
		return _logLatencyToFile;
	}
	
	/**
	 *  Name of the latency log file.
	 *
	 * @return the string
	 */
	public String latencyLogFilename()
	{
		return _latencyLogFilename;
	}
	
	/**
	 *  Name of the summary log file.
	 *
	 * @return the string
	 */
	public String summaryFilename()
	{
		return _summaryFilename;
	}
	
	/**
	 *  Name of the statistics log file.
	 *
	 * @return the string
	 */
	public String statsFilename()
	{
		return _statsFilename;
	}
	
	/**
	 *  Controls how often statistics are written.
	 *
	 * @return the int
	 */
	public int	writeStatsInterval()
	{
		return _writeStatsInterval;
	}
	
	/**
	 *  Controls whether stats appear on the screen.
	 *
	 * @return true, if successful
	 */
	public boolean displayStats()
	{
		return _displayStats;
	}

	/**
	 *  Rate at which the consumer will send out item requests.
	 *
	 * @return the int
	 */
	public int itemRequestsPerSec()
	{
		return _itemRequestsPerSec;
	}

	/**
	 *  Whether to request all items as snapshots.
	 *
	 * @return true, if successful
	 */
	public boolean requestSnapshots()
	{
		return _requestSnapshots;
	}

	/**
	 *  Username used when logging in.
	 *
	 * @return the string
	 */
	public String username()
	{
		return _username;
	}
	
	/**
	 *  Name of service to request items from.
	 *
	 * @return the string
	 */
	public String serviceName()
	{
		return _serviceName;
	}
	
	/**
	 *  Control if encoding item request with service id.
	 *
	 * @return true, if successful
	 */
	public boolean useServiceId()
	{
		return _useServiceId;
	}
	
	/**
	 *  Number of items to request.
	 *
	 * @return the int
	 */
	public int itemRequestCount()
	{
		return _itemRequestCount;
	}
	
	/**
	 *  Number of items common to all connections, if using multiple connections.
	 *
	 * @return the int
	 */
	public int commonItemCount()
	{
		return _commonItemCount;
	}
	
	/**
	 *  Number of posts to send per second.
	 *
	 * @return the int
	 */
	public int postsPerSec()
	{
		return _postsPerSec;
	}
	
	/**
	 *  Number of latency posts to send per second.
	 *
	 * @return the int
	 */
	public int latencyPostsPerSec()
	{
		return _latencyPostsPerSec;
	}

	/**
	 *  Number of generic messages to send per second.
	 *
	 * @return the int
	 */
	public int genMsgsPerSec()
	{
		return _genMsgsPerSec;
	}
	
	/**
	 *  Number of latency generic messages to send per second.
	 *
	 * @return the int
	 */
	public int latencyGenMsgsPerSec()
	{
		return _latencyGenMsgsPerSec;
	}
	
	/**
	 *  Number of requests to send per tick.
	 *
	 * @return the int
	 */
	public int requestsPerTick()
	{
		return _requestsPerTick;
	}
	
	/**
	 *  The remainder of number of requests to send per tick.
	 *
	 * @return the int
	 */
	public int requestsPerTickRemainder()
	{
		return _requestsPerTickRemainder;
	}
	
	/**
	 *  At startup, prime the JVM to optimize code by requesting a snapshot of all items before opening the streaming items.
	 *
	 * @return true, if successful
	 */
	public boolean primeJVM()
	{
		return _primeJVM;
	}
	
	/**
	 *  Converts configuration parameters to a string.
	 *
	 * @return the string
	 */
	public String toString()
	{
		return _configString;
	}

    /**
     *  Use the EMA API_DISPATCH model instead of the EMA USER_DISPATCH model for sending and receiving.
     *
     * @return true, if successful
     */
    public boolean useUserDispatch()
    {
        return _useUserDispatch;
    }
    
    /**
     *  Turn on the EMA data load downcast feature during decoding response payload.
     *
     * @return true, if successful
     */
    public boolean downcastDecoding()
    {
        return _downcastDecoding;
    }
}
