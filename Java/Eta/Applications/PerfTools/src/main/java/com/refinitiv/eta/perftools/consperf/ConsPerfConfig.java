package com.refinitiv.eta.perftools.consperf;

import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.shared.CommandLine;
import com.refinitiv.eta.transport.ConnectionTypes;

/** Provides configuration that is not specific to any particular handler. */
public class ConsPerfConfig
{
	private String _configString;
	private static final int DEFAULT_THREAD_COUNT = 1;
	private int _maxThreads;

	/* APPLICATION configuration */
	private int	_steadyStateTime;			/* Time application runs before exiting. */
	private int	_delaySteadyStateCalc;			/* Time before the latency is calculated. */
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

	/* CONNECTION configuration */
	private int	_connectionType;			/* Type of connection. */
	private String _hostName;				/* hostName, if using Transport.connect(). */
	private String _portNo;					/* Port number. */
	private String _interfaceName;			/* Name of interface. */
	private int _guaranteedOutputBuffers;	/* Guaranteed Output Buffers. */
	private int _numInputBuffers;			/* Input Buffers. */
	private int _sendBufSize;				/* System Send Buffer Size. */
	private int _recvBufSize;				/* System Receive Buffer Size. */
	private int _highWaterMark;				/* sets the point which will cause ETA to automatically flush */
	private boolean _tcpNoDelay;			/* Enable/Disable Nagle's algorithm. */

	private boolean _requestSnapshots;		/* Whether to request all items as snapshots. */

	private String _username;				/* Username used when logging in. */
	private String _serviceName;			/* Name of service to request items from. */
	private int _itemRequestCount;			/* Number of items to request. See -itemCount. */
	private int _commonItemCount;			/* Number of items common to all connections, if using multiple connections. */
	private int _postsPerSec;				/* Number of posts to send per second. */
	private int _latencyPostsPerSec;		/* Number of latency posts to send per second. */
	private int _genMsgsPerSec;				/* Number of generic msgs to send per second. */
	private int _latencyGenMsgsPerSec;		/* Number of latency generic msgs to send per second. */

	private int _requestsPerTick;			/* Number of requests to send per tick */
	private int _requestsPerTickRemainder;	/* The remainder of number of requests to send per tick */
	
	private boolean _primeJVM;				/* At startup, prime the JVM to optimize code by requesting a snapshot of all items before opening the streaming items. */
	private boolean _useReactor;            /* Use the VA Reactor instead of the ETA Channel for sending and receiving. */
	private boolean _useWatchlist;          /* Use the VA Reactor watchlist instead of the ETA Channel for sending and receiving. */
	private boolean _useTunnel;          	/* Use the VA Reactor tunnel stream instead of the ETA Channel for sending and receiving. */
	private boolean _tunnelAuth;            /* Use to request authentication when opening a tunnel stream. */
	private int _tunnelStreamOutputBuffers;	/* Tunnel Stream Guaranteed Output Buffers. */
	private boolean _tunnelStreamBufsUsed;  /* Control whether to print tunnel Stream buffers usage. */
	private boolean _busyRead;              /* If set, the application will continually read rather than using notification. */
	private String  _protocolList;          /* List of supported WS sub-protocols in order of preference(',' | white space delineated) */
	private int _encryptedConnType = ConnectionTypes.SOCKET; /* Encrypted connection type */
	private String _keyfile = null;			/* Keyfile for encrypted connection */
	private String _keypasswd = null;		/* password for encrypted keyfile */

    {
        CommandLine.programName("ConsPerf");
        CommandLine.addOption("steadyStateTime", 300, "Time consumer will run the steady-state portion of the test. Also used as a timeout during the startup-state portion");
        CommandLine.addOption("delaySteadyStateCalc", 0, "Time consumer will wait before calculate the latency (milliseconds)");
        CommandLine.addOption("tickRate", 1000, "Ticks per second");
        CommandLine.addOption("threads", DEFAULT_THREAD_COUNT, "Number of threads that handle connections");
        CommandLine.addOption("itemFile", "350k.xml", "Name of the file to get item names from");
        CommandLine.addOption("msgFile", "MsgData.xml", "Name of the file that specifies the data content in messages");
        CommandLine.addOption("latencyFile", "", "Base name of file for logging latency");
        CommandLine.addOption("summaryFile", "ConsSummary.out", "Name of file for logging summary info");
        CommandLine.addOption("statsFile", "ConsStats", "Base name of file for logging periodic statistics");
        CommandLine.addOption("writeStatsInterval", 5, "Controls how often stats are written to the file");
        CommandLine.addOption("noDisplayStats", false, "Stop printout of stats to screen");
        CommandLine.addOption("requestRate", 500000, "Rate at which to request items");
        CommandLine.addOption("connType", "socket", "Type of connection");
        CommandLine.addOption("h", "localhost", "Name of host to connect to");
        CommandLine.addOption("p", "14002", "Port number to connect to");
        CommandLine.addOption("if", "", "Name of network interface to use");
        CommandLine.addOption("outputBufs", 5000, "Number of output buffers(configures guaranteedOutputBuffers in ETA connection)");
        CommandLine.addOption("inputBufs", 15, "Number of input buffers(configures numInputBufs in ETA connection)");
        CommandLine.addOption("sendBufSize", 0, "System Send Buffer Size(configures sysSendBufSize in ConnectOptions)");
        CommandLine.addOption("recvBufSize", 0, "System Receive Buffer Size(configures sysRecvBufSize in ConnectOptions)");
        CommandLine.addOption("highWaterMark", 0, "Sets the point that causes ETA to automatically flush");
        CommandLine.addOption("tcpDelay", false, "Turns off tcp_nodelay in BindOptions, enabling Nagle's");
        CommandLine.addOption("snapshot", false, "Snapshot test, request all items as non-streaming");
        CommandLine.addOption("uname", "", "Username to use in login request");
        CommandLine.addOption("serviceName", "DIRECT_FEED", "Name of service to request items from");
        CommandLine.addOption("itemCount", 100000, "Number of items to request");
        CommandLine.addOption("commonItemCount", 0, "Number of items common to all consumers, if using multiple connections");
        CommandLine.addOption("postingRate", 0, "Rate at which to send post messages");
        CommandLine.addOption("postingLatencyRate", 0, "Rate at which to send latency post messages");
        CommandLine.addOption("genericMsgRate", 0, "Rate at which to send generic messages");
        CommandLine.addOption("genericMsgLatencyRate", 0, "Rate at which to send latency generic messages");
        CommandLine.addOption("primeJVM", false, "At startup, prime the JVM to optimize code by requesting a snapshot of all items before opening the streaming items");
        CommandLine.addOption("reactor", false, "Use the VA Reactor instead of the ETA Channel for sending and receiving");
        CommandLine.addOption("watchlist", false, "Use the VA Reactor watchlist instead of the ETA Channel for sending and receiving");
        CommandLine.addOption("tunnel", false, "Use the VA Reactor tunnel stream instead of the ETA Channel for sending and receiving");
        CommandLine.addOption("tunnelAuth", false, "If set, consumer to request authentication when opening a tunnel stream");
        CommandLine.addOption("tunnelStreamOutputBufs", 5000, "Number of output buffers(configures guaranteedOutputBuffers in Tunnel Stream)");
        CommandLine.addOption("tunnelStreamBuffersUsed", false, "Print stats of buffers used by tunnel stream");
        CommandLine.addOption("busyRead", false, "If set, the application will continually read rather than using notification.");
        CommandLine.addOption("pl", "", "List of supported WS sub-protocols in order of preference(',' | white space delineated)");
        CommandLine.addOption("keyfile", "", "Keystore file location and name");
        CommandLine.addOption("keypasswd", "", "Keystore password");        
        CommandLine.addOption("encryptedConnectionType", "", "Specifies the encrypted connection type that will be used by the consumer.  Possible values are 'socket', 'websocket' or 'http'");
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
        	System.exit(CodecReturnCodes.FAILURE);
        }

    	_maxThreads = maxThreads;
    	_msgFilename = CommandLine.value("msgFile");
    	_itemFilename = CommandLine.value("itemFile");
    	_logLatencyToFile = false;
    	_latencyLogFilename = CommandLine.value("latencyFile");
    	_summaryFilename = CommandLine.value("summaryFile");
    	_statsFilename = CommandLine.value("statsFile");
    	_hostName = CommandLine.value("h");
    	_portNo = CommandLine.value("p");
    	_interfaceName = CommandLine.value("if");
    	_username = CommandLine.value("uname");
    	_serviceName = CommandLine.value("serviceName");
    	_protocolList = CommandLine.value("pl");
    	_displayStats = !CommandLine.booleanValue("noDisplayStats");
    	_tcpNoDelay = !CommandLine.booleanValue("tcpDelay");
    	_requestSnapshots = CommandLine.booleanValue("snapshot");
    	_primeJVM = CommandLine.booleanValue("primeJVM");
    	_useReactor = CommandLine.booleanValue("reactor");
        _useWatchlist = CommandLine.booleanValue("watchlist");
        _useTunnel = CommandLine.booleanValue("tunnel");
        _tunnelAuth = CommandLine.booleanValue("tunnelAuth");
        _busyRead = CommandLine.booleanValue("busyRead");
        _tunnelStreamOutputBuffers = CommandLine.intValue("tunnelStreamOutputBufs");
        _tunnelStreamBufsUsed = CommandLine.booleanValue("tunnelStreamBuffersUsed");
        try
        {
        	_steadyStateTime = CommandLine.intValue("steadyStateTime");
        	_delaySteadyStateCalc = CommandLine.intValue("delaySteadyStateCalc");
        	_ticksPerSec = CommandLine.intValue("tickRate");
        	_threadCount = CommandLine.intValue("threads");
        	_writeStatsInterval = CommandLine.intValue("writeStatsInterval");
        	_itemRequestsPerSec = CommandLine.intValue("requestRate");
            if("socket".equals(CommandLine.value("connType")))
            {
            	_connectionType = ConnectionTypes.SOCKET;
            }
            else if("websocket".equals(CommandLine.value("connType")))
            {
            	_connectionType = ConnectionTypes.WEBSOCKET;
            }
            else if("encrypted".equals(CommandLine.value("connType")))
            {
            	_connectionType = ConnectionTypes.ENCRYPTED;
            	_keyfile = CommandLine.value("keyfile");
            	_keypasswd = CommandLine.value("keypasswd");
            	
            	if("socket".equals(CommandLine.value("encryptedConnectionType")))
                {
                	_encryptedConnType = ConnectionTypes.SOCKET;
                }
            	else if("websocket".equals(CommandLine.value("encryptedConnectionType")))
                {
                	_encryptedConnType = ConnectionTypes.WEBSOCKET;
                }
            	else if("http".equals(CommandLine.value("encryptedConnectionType")))
                {
                	_encryptedConnType = ConnectionTypes.HTTP;
                }
            	else
            	{
            		System.err.println("Config Error: Only socket, websocket or http encrypted connection type is supported.\n");
                	System.out.println(CommandLine.optionHelpString());
                	System.exit(-1);
            	}
            }
            else
            {
            	System.err.println("Config Error: Only socket, websocket or encrypted connection type is supported.\n");
            	System.out.println(CommandLine.optionHelpString());
            	System.exit(-1);
            }
        	_guaranteedOutputBuffers = CommandLine.intValue("outputBufs");
        	_numInputBuffers = CommandLine.intValue("inputBufs");
        	_sendBufSize = CommandLine.intValue("sendBufSize");
        	_recvBufSize = CommandLine.intValue("recvBufSize");
        	_highWaterMark = CommandLine.intValue("highWaterMark");
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
        
        if (_connectionType != ConnectionTypes.SOCKET && _connectionType != ConnectionTypes.WEBSOCKET && _connectionType != ConnectionTypes.ENCRYPTED)
        {
			System.err.println("Config Error: Application only supports SOCKET, WEBSOCKET and ENCRYPTED connection types.\n");
			System.out.println(CommandLine.optionHelpString());
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
		
		if(_useTunnel && !_useReactor)
		{
			System.err.println("Config error: The -reactor must be set in order to send and receive messages via tunnel stream.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		if (_delaySteadyStateCalc < 0 || _delaySteadyStateCalc > 30000)
		{
			System.err.println("Config error: Time before the latency is calculated should not be less than 0 or greater than 30000.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		if ((_delaySteadyStateCalc / 1000) > _steadyStateTime)
		{
			System.err.println("Config Error: Time before the latency is calculated should be less than Steady State Time.\n");
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
	    String reactorWatchlistUsageString;

	    if (_useWatchlist)
	    {
	        reactorWatchlistUsageString = "Reactor and Watchlist";
	    }
	    else if (_useReactor)
	    {
	        reactorWatchlistUsageString = "Reactor";
	    }
	    else
	    {
	        reactorWatchlistUsageString = "None";
	    }

		_configString = "--- TEST INPUTS ---\n\n" +
            "          Steady State Time: " + _steadyStateTime + " sec\n" + 
            "    Delay Steady State Time: " + _delaySteadyStateCalc + " msec\n" + 
            "            Connection Type: " + ConnectionTypes.toString(_connectionType) + "\n" +
            ((_connectionType == ConnectionTypes.ENCRYPTED) ? "" : 
            " Encrypted Connection Type: " + ConnectionTypes.toString(_encryptedConnType) + "\n" + 
            "                   keyfile: " + _keyfile + "\n") +
            "                   Hostname: " + _hostName + "\n" +
            "                       Port: " + _portNo + "\n" +
            "                    Service: " + _serviceName + "\n" +
            "               Thread Count: " + _threadCount + "\n" +
            "             Output Buffers: " + _guaranteedOutputBuffers + "\n" +
            "              Input Buffers: " + _numInputBuffers + "\n" +
            "           Send Buffer Size: " + _sendBufSize + ((_sendBufSize > 0) ? " bytes" : "(use default)") + "\n" +
            "           Recv Buffer Size: " + _recvBufSize + ((_recvBufSize > 0) ? " bytes" : "(use default)") + "\n" +
            "            High Water Mark: " + _highWaterMark + ((_highWaterMark > 0) ? " bytes" : "(use default)") + "\n" +
            "             Interface Name: " + (_interfaceName.length() > 0 ? _interfaceName : "(use default)") + "\n" +
            "                Tcp_NoDelay: " + (_tcpNoDelay ? "Yes" : "No") + "\n" +
            "                   Username: " + (_username.length() > 0 ? _username : "(use system login name)") + "\n" +
            "                 Item Count: " + _itemRequestCount + "\n" +
            "          Common Item Count: " + _commonItemCount + "\n" +
            "               Request Rate: " + _itemRequestsPerSec + "\n" +
            "          Request Snapshots: " + (_requestSnapshots ? "Yes" : "No") + "\n" +
            "               Posting Rate: " + _postsPerSec + "\n" +
            "       Latency Posting Rate: " + _latencyPostsPerSec + "\n" +
            "           Generic Msg Rate: " + _genMsgsPerSec + "\n" +
            "   Generic Msg Latency Rate: " + _latencyGenMsgsPerSec + "\n" +
            "                  Item File: " + _itemFilename + "\n" +
            "                  Data File: " + _msgFilename + "\n" +
            "               Summary File: " + _summaryFilename + "\n" +
            "                 Stats File: " + _statsFilename + "\n" +
            "           Latency Log File: " + (_latencyLogFilename.length() > 0 ? _latencyLogFilename : "(none)") + "\n" +
            "                  Tick Rate: " + _ticksPerSec + "\n" +
	        "                  Prime JVM: " + (_primeJVM ? "Yes" : "No") + "\n" +
            "    Reactor/Watchlist Usage: " + reactorWatchlistUsageString + "\n" +
            "              Tunnel Stream: " + (_useTunnel ? "Yes" : "No") + "\n" +
            "      Tunnel Authentication: " + (_tunnelAuth ? "Yes" : "No") + "\n" +
            "              Protocol list: " + (_protocolList.isEmpty() ? "Not specified" : _protocolList) + "\n" +
            "TunnelStream Output Buffers: " + _tunnelStreamOutputBuffers + "\n" +
            "Print TunnelStream Bufs Used: " + (_tunnelStreamBufsUsed ? "Yes" : "No") + "\n" +
            "                  Busy Read: " + (_busyRead ? "Yes" : "No") + "\n";
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
	 *  Time before the latency is calculated.
	 *
	 * @return the int
	 */
	public int	delaySteadyStateCalc()
	{
		return _delaySteadyStateCalc;
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

	/* CONNECTION configuration */
	/**
	 *  Type of connection.
	 *
	 * @return the int
	 */
	public int	connectionType()
	{
		return _connectionType;
	}
	
	/**
	 *  hostName, if using Channel.connect().
	 *
	 * @return the string
	 */
	public String hostName()
	{
		return _hostName;
	}
	
	/**
	 *  Port number.
	 *
	 * @return the string
	 */
	public String portNo()
	{
		return _portNo;
	}
	
	/**
	 *  Name of interface.
	 *
	 * @return the string
	 */
	public String interfaceName()
	{
		return _interfaceName;
	}
	
	/** 
	 * Encrypted connection type
	 * 
	 * @return the encrypted connection type
	 */
	public int encryptedConnectionType()
	{
		return _encryptedConnType;
	}
	
	/** 
	 * keyfile
	 * 
	 * @return the keyfile
	 */
	public String keyfile()
	{
		return _keyfile;
	}
	
	/** 
	 * keyfile password
	 * 
	 * @return the keyfile password
	 */
	public String keypasswd()
	{
		return _keypasswd;
	}
	
	
	/**
	 *  Guaranteed Output Buffers.
	 *
	 * @return the int
	 */
	public int guaranteedOutputBuffers()
	{
		return _guaranteedOutputBuffers;
	}
	
	/**
	 *  TunnelStream Guaranteed Output Buffers.
	 *
	 * @return the int
	 */
	public int tunnelStreamGuaranteedOutputBuffers()
	{
		return _tunnelStreamOutputBuffers;
	}
	
	/**
	 *  Control to print TunnelStream Usage Buffers.
	 *
	 * @return the boolean
	 */
	public boolean tunnelStreamBufsUsed()
	{
		return _tunnelStreamBufsUsed;
	}
	
	/**
	 *  Input Buffers.
	 *
	 * @return the int
	 */
	public int numInputBuffers()
	{
		return _numInputBuffers;
	}
	
	/**
	 *  System Send Buffer Size.
	 *
	 * @return the int
	 */
	public int sendBufSize()
	{
		return _sendBufSize;
	}
	
	/**
	 *  System Receive Buffer Size.
	 *
	 * @return the int
	 */
	public int recvBufSize()
	{
		return _recvBufSize;
	}

	/**
	 *  Sets the point that causes ETA to automatically flush.
	 *
	 * @return the int
	 */
	public int highWaterMark()
	{
		return _highWaterMark;
	}
	
	/**
	 *  Enable/Disable Nagle's algorithm.
	 *
	 * @return true, if successful
	 */
	public boolean tcpNoDelay()
	{
		return _tcpNoDelay;
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
     *  Use the VA Reactor instead of the ETA Channel for sending and receiving.
     *
     * @return true, if successful
     */
    public boolean useReactor()
    {
        return _useReactor;
    }

    /**
     *  Use the VA Reactor watchlist instead of the ETA Channel for sending and receiving.
     *
     * @return true, if successful
     */
    public boolean useWatchlist()
    {
        return _useWatchlist;
    }
    
    /**
     *  Use the VA Reactor tunnel stream instead of the ETA Channel for sending and receiving.
     *
     * @return true, if successful
     */
    public boolean useTunnel()
    {
        return _useTunnel;
    }
    
    /**
     *  Use to request authentication when opening a tunnel stream.
     *
     * @return true, if successful
     */
    public boolean tunnelAuth()
    {
        return _tunnelAuth;
    }
    
    /**
     *  If set, the application will continually read rather than using notification.
     *
     * @return true, if successful
     */
    public boolean busyRead()
    {
        return _busyRead;
    }
    
    /**
	 * The websocket sub-protocol list specified by users.
	 * 
	 * @return the sub-protocol list 
	 */
	public String protocolList()
	{
		return _protocolList;
	}
}
