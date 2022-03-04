/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.shared.CommandLine;
import com.refinitiv.eta.transport.ConnectionTypes;

/**
 * Provides configuration option for the non-interactive provider performance
 * application.
 */
public class NIProvPerfConfig
{
	private static String _configString, _connectionString;
	private static final int DEFAULT_THREAD_COUNT = 1;
    private static final int ALWAYS_SEND_LATENCY_UPDATE = -1;
    
	/* APPLICATION configuration */
	private static int	_runTime;					/* Time application runs before exiting. */
	private static String _summaryFilename;			/* Name of the summary log file. */
	private static int	_writeStatsInterval;		/* Controls how often statistics are written. */
	private static boolean _displayStats;			/* Controls whether stats appear on the screen. */

	/* CONNECTION configuration */
	private static String _connectionType;			/* Type of connection. */
	private static String _encryptedConnectionType; /* Type of encrypted connection */
	private static String _hostName;				/* hostName, if using Transport.connect(). */
	private static String _portNo;					/* Port number. */
	private static String _interfaceName;			/* Name of interface. */
	private static String _sendAddress;				/* Send address */
	private static String _sendPort;				/* Send port */
	private static String _recvAddress;				/* Receive address */
	private static String _recvPort;				/* Receive port */
	private static String _unicastPort;				/* Unicast port */
	private static String _username;				/* User name used when logging in. */
	private static boolean _tcpNoDelay;				/* Enable/Disable Nagle's algorithm. */
	private static int _guaranteedOutputBuffers;	/* Guaranteed Output Buffers. */
	private static int	_maxFragmentSize;			/* Maximum Fragment Size. */
	private static int _sendBufSize;				/* System Send Buffer Size. */
	private static int _recvBufSize;				/* System Receive Buffer Size. */
	private static int _highWaterMark;				/* sets the point which will cause ETA to automatically flush */
	private static int _itemPublishCount;			/* Number of items to publish non-interactively. See -itemCount. */
	private static int _commonItemCount;			/* Number of items common to all providers, if using multiple connections. */
    private static boolean _useReactor;             /* Use the VA Reactor instead of the ETA Channel for sending and receiving. */
	private static String _keyfile;                 /* Keyfile used for encrypted connection */
	private static String _keypasswd;               /* Key password used for encrypted connection */

	static
    {
    	CommandLine.addOption("runTime", 360, "Time application runs before exiting");
    	CommandLine.addOption("summaryFile", "NIProvSummary.out", "Name of file for logging summary info");
    	CommandLine.addOption("writeStatsInterval", 5, "Controls how often stats are written to the file");
    	CommandLine.addOption("noDisplayStats", false, "Stop printout of stats to screen");
    	CommandLine.addOption("connType", "socket", "Type of connection (\"socket\", \"encrypted\", \"reliableMCast\")");
		CommandLine.addOption("encryptedConnectionType", "socket", "Type of encrypted connection (\"socket\", \"http\")");
    	CommandLine.addOption("h", "localhost", "Name of host for socket connection");
    	CommandLine.addOption("p", "14003", "Port number for socket connection");
    	CommandLine.addOption("if", "", "Name of network interface to use");
        CommandLine.addOption("sa", "", "Send address for reliable multicast");
        CommandLine.addOption("sp", "", "Send port for reliable multicast");
        CommandLine.addOption("ra", "", "Receive address for reliable multicast");
        CommandLine.addOption("rp", "", "Receive port for reliable multicast");
        CommandLine.addOption("u", "", "Unicast port for reliable multicast");
    	CommandLine.addOption("uname", "", "Username to use in login request");
    	CommandLine.addOption("tcpDelay", false, "Turns off tcp_nodelay in ConnectOptions, enabling Nagle's");
    	CommandLine.addOption("outputBufs", 5000, "Number of output buffers(configures guaranteedOutputBuffers in ETA connection)");
    	CommandLine.addOption("maxFragmentSize", 6144, "Maximum Fragment Size");
    	CommandLine.addOption("sendBufSize", 0, "System Send Buffer Size(configures sysSendBufSize in ConnectOptions)");
    	CommandLine.addOption("recvBufSize", 0, "System Receive Buffer Size(configures sysRecvBufSize in ConnectOptions)");
    	CommandLine.addOption("highWaterMark", 0, "Sets the point that causes ETA to automatically flush");
    	CommandLine.addOption("itemCount", 100000, "Number of items to publish non-interactively");
    	CommandLine.addOption("commonItemCount", 0, "Number of items common to all providers, if using multiple connections");
        CommandLine.addOption("reactor", false, "Use the VA Reactor instead of the ETA Channel for sending and receiving");

    	// ProviderPerfConfig
    	CommandLine.addOption("threads", DEFAULT_THREAD_COUNT, "Number of non-interactive provider threads to create");
    	CommandLine.addOption("statsFile", "NIProvStats", "Base name of file for logging periodic statistics");
    	CommandLine.addOption("itemFile", "350k.xml", "Name of the file to get item names from");
    	CommandLine.addOption("msgFile", "MsgData.xml", "Name of the file that specifies the data content in messages");
    	CommandLine.addOption("updateRate", 100000, "Total update rate per second(includes latency updates)");
    	CommandLine.addOption("refreshBurstSize", 10, "Number of refreshes to send in a burst(controls granularity of time-checking)");
    	CommandLine.addOption("latencyUpdateRate", 10, "Total latency update rate per second");
    	CommandLine.addOption("tickRate", 1000, "Ticks per second");
    	CommandLine.addOption("maxPackCount", 1, "Number of messages packed into a given buffer");
    	CommandLine.addOption("packBufSize", 6000, "Size of packable buffer, if packing");
    	CommandLine.addOption("directWrite", false, "Turns on direct write flag");
    	CommandLine.addOption("serviceName", "DIRECT_FEED", "Name of the provided service");
    	CommandLine.addOption("serviceId", 1, "ID of the provided service");
		CommandLine.addOption("keyfile", "", "Keyfile used for encrypted connection");
		CommandLine.addOption("keypasswd", "", "Key password used for encrypted connection");
    }
	
	/* NIProvPerfConfig cannot be instantiated */
    private NIProvPerfConfig()
    {
        
    }
    
    /**
     *  Parses command-line arguments to fill in the application's configuration structures.
     *
     * @param args the args
     */
	public static void init(String[] args)
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

    	/* Don't need to provide an open limit.  Our encode logic will skip the OpenLimit entry if it
    	 * is set to zero. */
        ProviderPerfConfig.openLimit(0);

        /* Overwrite programName here since ProviderPerfConfig and
         * NIProvPerfConfig share same CommandLine. */
    	CommandLine.programName("NIProvPerf");    	
        ProviderPerfConfig.itemFilename(CommandLine.value("itemFile"));
        ProviderPerfConfig.msgFilename(CommandLine.value("msgFile"));
        _summaryFilename = CommandLine.value("summaryFile");
        ProviderPerfConfig.statsFilename(CommandLine.value("statsFile"));
        _displayStats = !CommandLine.booleanValue("noDisplayStats");
        _hostName = CommandLine.value("h");
        _portNo = CommandLine.value("p");
        _interfaceName = CommandLine.value("if");
        _sendAddress = CommandLine.value("sa");
    	_sendPort = CommandLine.value("sp");
    	_recvAddress = CommandLine.value("ra");
    	_recvPort = CommandLine.value("rp");
    	_unicastPort = CommandLine.value("u");
        _username = CommandLine.value("uname");
        _tcpNoDelay = !CommandLine.booleanValue("tcpDelay");        
        ProviderPerfConfig.directWrite(CommandLine.booleanValue("directWrite"));
        ProviderPerfConfig.serviceName(CommandLine.value("serviceName"));
        _useReactor = CommandLine.booleanValue("reactor");
        _keyfile = CommandLine.value("keyfile");
        _keypasswd = CommandLine.value("keypasswd");
        _encryptedConnectionType = CommandLine.value("encryptedConnectionType");
        
        try
        {
            _runTime = CommandLine.intValue("runTime");
            ProviderPerfConfig.ticksPerSec(CommandLine.intValue("tickRate"));
            ProviderPerfConfig.threadCount(CommandLine.intValue("threads") > 1 ? CommandLine.intValue("threads") : 1);
            _writeStatsInterval = CommandLine.intValue("writeStatsInterval");
            ProviderPerfConfig.totalBuffersPerPack(CommandLine.intValue("maxPackCount"));
            ProviderPerfConfig.packingBufferLength(CommandLine.intValue("packBufSize"));
        	ProviderPerfConfig.refreshBurstSize(CommandLine.intValue("refreshBurstSize"));
        	ProviderPerfConfig.updatesPerSec(CommandLine.intValue("updateRate"));
            if("all".equals(CommandLine.value("latencyUpdateRate")))
            	ProviderPerfConfig.latencyUpdateRate(ALWAYS_SEND_LATENCY_UPDATE);
            else
            	ProviderPerfConfig.latencyUpdateRate(CommandLine.intValue("latencyUpdateRate"));
            _connectionType = CommandLine.value("connType");
            if(!"socket".equals(_connectionType) &&
               !"reliableMCast".equals(_connectionType) && !"encrypted".equals(_connectionType))
            {
            	System.err.println("Config Error: Only socket, encrypted or reliableMCast connection type is supported.\n");
            	System.out.println(CommandLine.optionHelpString());
            	System.exit(-1);
            }
            _guaranteedOutputBuffers = CommandLine.intValue("outputBufs");
            _maxFragmentSize = CommandLine.intValue("maxFragmentSize");
            _sendBufSize = CommandLine.intValue("sendBufSize");
            _recvBufSize = CommandLine.intValue("recvBufSize");
            _highWaterMark = CommandLine.intValue("highWaterMark");
            _itemPublishCount = CommandLine.intValue("itemCount");
            _commonItemCount = CommandLine.intValue("commonItemCount");
            ProviderPerfConfig.serviceId(CommandLine.intValue("serviceId"));        	
        }
        catch (NumberFormatException ile)
        {
        	System.err.println("Invalid argument, number expected.\t");
        	System.err.println(ile.getMessage());
        	System.err.println();
        	System.err.println(CommandLine.optionHelpString());
        	System.exit(-1);
        }
        
        /* Conditions */
        
    	if (ProviderPerfConfig.threadCount() < 1)
    	{
    		System.err.println("Config Error: Thread count cannot be less than 1.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
    	} 
        
    	if (ProviderPerfConfig.ticksPerSec() < 1)
    	{
    		System.err.println("Config Error: Tick rate cannot be less than 1.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
    	} 
        
    	if (_commonItemCount > _itemPublishCount / ProviderPerfConfig.threadCount())
    	{
    		System.err.printf("Config Error: Common item count (%d) is greater than total item count per thread (%d).\n",
    				_commonItemCount, 
    				_itemPublishCount / ProviderPerfConfig.threadCount());
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
    	}

    	if (_commonItemCount > _itemPublishCount)
    	{
    		System.err.printf("Config Error: Common item count is greater than total item count.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
    	}
		
    	if (ProviderPerfConfig.latencyUpdateRate() > ProviderPerfConfig.updatesPerSec())
    	{
    		System.err.println("Config Error: Latency update rate cannot be greater than total update rate. \n\n");
    		System.exit(-1);
    	}

    	if (ProviderPerfConfig.latencyUpdateRate() > ProviderPerfConfig.ticksPerSec())
    	{
    		System.err.println("Config Error: Latency update rate cannot be greater than total ticks per second. \n\n");
    		System.exit(-1);
    	}

    	if (ProviderPerfConfig.updatesPerSec() != 0 && ProviderPerfConfig.updatesPerSec() < ProviderPerfConfig.ticksPerSec())
    	{
    		System.err.println("Config Error: Update rate cannot be less than total ticks per second(unless it is zero).\n\n");
    		System.exit(-1);
    	}

    	if (ProviderPerfConfig.totalBuffersPerPack() < 1)
    	{
    		System.err.println("Config Error: Cannot specify less than 1 buffer per pack.\n\n");
    		System.exit(-1);
    	}

    	if (ProviderPerfConfig.totalBuffersPerPack() > 1 && ProviderPerfConfig.packingBufferLength() == 0)
    	{
    		System.err.println("Config Error: -maxPackCount set but -packBufSize is zero.\n\n");
    		System.exit(-1);
    	}

        ProviderPerfConfig.updatesPerTick(ProviderPerfConfig.updatesPerSec() / ProviderPerfConfig.ticksPerSec());
        ProviderPerfConfig.updatesPerTickRemainder(ProviderPerfConfig.updatesPerSec() % ProviderPerfConfig.ticksPerSec());
        
    	createConfigString();
	}
    
	/* Create config string. */
	private static void createConfigString()
	{
		if ("reliableMCast".equals(_connectionType))
		{
			_connectionString =
				"   Multicast Connection: (send " + _sendAddress + ":" + _sendPort + ", recv " + _recvAddress + ":" + _recvPort + ", unicast " + _unicastPort + ")\n";
		}
		else
		{
			_connectionString = "               Hostname: " + _hostName + "\n" +
			                    "                   Port: " + _portNo + "\n";

		}
		_configString = "--- TEST INPUTS ---\n\n" +
				"               Run Time: " + _runTime + " sec\n" + 
				"        Connection Type: " + _connectionType + "\n" +
				_connectionString +
				"           Thread Count: " + ProviderPerfConfig.threadCount() + "\n" +
				"         Output Buffers: " + _guaranteedOutputBuffers + "\n" +
				"      Max Fragment Size: " + _maxFragmentSize + "\n" +
				"       Send Buffer Size: " + _sendBufSize + ((_sendBufSize > 0) ? " bytes" : "(use default)") + "\n" +
				"       Recv Buffer Size: " + _recvBufSize + ((_recvBufSize > 0) ? " bytes" : "(use default)") + "\n" +
				"        High Water Mark: " + _highWaterMark + ((_highWaterMark > 0) ? " bytes" : "(use default)") + "\n" +
				"         Interface Name: " + (_interfaceName.length() > 0 ? _interfaceName : "(use default)") + "\n" +
				"               Username: " + (_username.length() > 0 ? _username : "(use system login name)") + "\n" +
				"            Tcp_NoDelay: " + (_tcpNoDelay ? "Yes" : "No") + "\n" +
				"             Item Count: " + _itemPublishCount + "\n" +
				"      Common Item Count: " + _commonItemCount + "\n" +
				"              Tick Rate: " + ProviderPerfConfig.ticksPerSec() + "\n" +
				"      Use Direct Writes: " + (ProviderPerfConfig.directWrite() ? "Yes" : "No") + "\n" +
				"           Summary File: " + _summaryFilename + "\n" +
				"             Stats File: " + ProviderPerfConfig.statsFilename() + "\n" +
				"   Write Stats Interval: " + _writeStatsInterval + "\n" +
				"          Display Stats: " + _displayStats + "\n" +
				"            Update Rate: " + ProviderPerfConfig.updatesPerSec() + "\n" +
				"    Latency Update Rate: " + ((ProviderPerfConfig.latencyUpdateRate() >= 0) ? ProviderPerfConfig.latencyUpdateRate() : ProviderPerfConfig.updatesPerSec()) + "\n" +
				"     Refresh Burst Size: " + ProviderPerfConfig.refreshBurstSize() + "\n" +
				"              Item File: " + ProviderPerfConfig.itemFilename() + "\n" +
				"              Data File: " + ProviderPerfConfig.msgFilename() + "\n" +
				"                Packing: " + ((ProviderPerfConfig.totalBuffersPerPack() > 1) ? "Yes(max " + ProviderPerfConfig.totalBuffersPerPack() + " per pack, " + ProviderPerfConfig.packingBufferLength() + " buffer size)": "No") + "\n" +
				"             Service ID: " + ProviderPerfConfig.serviceId() + "\n" +
				"           Service Name: " + ProviderPerfConfig.serviceName() + "\n" +
                "            Use Reactor: " + (_useReactor ? "Yes" : "No") + "\n\n";
	}

	/* APPLICATION configuration */
    /**
	 *  Time application runs before exiting.
	 *
	 * @return the int
	 */
	public static int runTime()
	{
		return _runTime;
	}
	
	/**
	 *  Main loop ticks per second.
	 *
	 * @return the int
	 */
	public static int ticksPerSec()
	{
		return ProviderPerfConfig.ticksPerSec();
	}
	
	/**
	 *  Number of threads that handle connections.
	 *
	 * @return the int
	 */
	public static int threadCount()
	{
		return ProviderPerfConfig.threadCount();
	}

	/**
	 *  Item List file. Provides a list of items to open.
	 *
	 * @return the string
	 */
	public static String itemFilename()
	{
		return ProviderPerfConfig.itemFilename();
	}
	
	/**
	 *  Data file. Describes the data to use when encoding messages.
	 *
	 * @return the string
	 */
	public static String msgFilename()
	{
		return ProviderPerfConfig.msgFilename();
	}

	/**
	 *  Name of the summary log file.
	 *
	 * @return the string
	 */
	public static String summaryFilename()
	{
		return _summaryFilename;
	}
	
	/**
	 *  Name of the statistics log file.
	 *
	 * @return the string
	 */
	public static String statsFilename()
	{
		return ProviderPerfConfig.statsFilename();
	}
	
	/**
	 *  Controls how often statistics are written.
	 *
	 * @return the int
	 */
	public static int writeStatsInterval()
	{
		return _writeStatsInterval;
	}
	
	/**
	 *  Controls whether stats appear on the screen.
	 *
	 * @return true, if successful
	 */
	public static boolean displayStats()
	{
		return _displayStats;
	}

	/**
	 *  How many messages are packed into a given buffer.
	 *
	 * @return the int
	 */
	public static int totalBuffersPerPack()
	{
		return	ProviderPerfConfig.totalBuffersPerPack();		
	}
	
	/**
	 *  Size of packable buffer, if packing.
	 *
	 * @return the int
	 */
	public static int packingBufferLength()
	{
		return	ProviderPerfConfig.packingBufferLength();	
	}
	
	/**
	 *  Total refresh rate per second.
	 *
	 * @return the int
	 */
	public static int refreshBurstSize()
	{
		return	ProviderPerfConfig.refreshBurstSize();		
	}
	
	/* CONNECTION configuration */
	/**
	 *  Type of connection.
	 *
	 * @return the int
	 */
	public static int connectionType()
	{
		int retVal = -1;

        if("socket".equals(_connectionType))
        {
        	retVal = ConnectionTypes.SOCKET;
        }
        else if ("reliableMCast".equals(_connectionType))
        {
        	retVal = ConnectionTypes.RELIABLE_MCAST;
        }
        else if ("encrypted".equals(_connectionType))
		{
			retVal = ConnectionTypes.ENCRYPTED;
		}
        
        return retVal;
	}

	/**
	 *  Type of encrypted connection.
	 *
	 * @return the int
	 */
	public static int encryptedConnectionType()
	{
		int retVal = -1;

		if("socket".equals(_encryptedConnectionType))
		{
			retVal = ConnectionTypes.SOCKET;
		}
		else if ("http".equals(_encryptedConnectionType))
		{
			retVal = ConnectionTypes.HTTP;
		}

		return retVal;
	}
	
	/**
	 *  hostName, if using Channel.connect().
	 *
	 * @return the string
	 */
	public static String hostName()
	{
		return _hostName;
	}
	
	/**
	 *  Port number.
	 *
	 * @return the string
	 */
	public static String portNo()
	{
		return _portNo;
	}
	
	/**
	 *  Name of interface.
	 *
	 * @return the string
	 */
	public static String interfaceName()
	{
		return _interfaceName;
	}
	
	/**
	 *  Send address.
	 *
	 * @return the string
	 */
	public static String sendAddress()
	{
		return _sendAddress;
	}
	
	/**
	 *  Send port.
	 *
	 * @return the string
	 */
	public static String sendPort()
	{
		return _sendPort;
	}
	
	/**
	 *  Receive address.
	 *
	 * @return the string
	 */
	public static String recvAddress()
	{
		return _recvAddress;
	}
	
	/**
	 *  Receive port.
	 *
	 * @return the string
	 */
	public static String recvPort()
	{
		return _recvPort;
	}
	
	/**
	 *  Unicast port.
	 *
	 * @return the string
	 */
	public static String unicastPort()
	{
		return _unicastPort;
	}
	
	/**
	 *  Guaranteed Output Buffers.
	 *
	 * @return the int
	 */
	public static int guaranteedOutputBuffers()
	{
		return _guaranteedOutputBuffers;
	}
	
	/**
	 *  System Send Buffer Size.
	 *
	 * @return the int
	 */
	public static int sendBufSize()
	{
		return _sendBufSize;
	}
	
	/**
	 *  System Receive Buffer Size.
	 *
	 * @return the int
	 */
	public static int recvBufSize()
	{
		return _recvBufSize;
	}

	/**
	 *  The point that causes ETA to automatically flush.
	 *
	 * @return the int
	 */
	public static int highWaterMark()
	{
		return _highWaterMark;
	}
	
	/**
	 *  The fragment size.
	 *
	 * @return the int
	 */
	public static int maxFragmentSize()
	{
		return _maxFragmentSize;
	}
	
	/**
	 *  Enable/Disable Nagle's algorithm.
	 *
	 * @return true, if successful
	 */
	public static boolean tcpNoDelay()
	{
		return _tcpNoDelay;
	}

	/**
	 *  Username used when logging in.
	 *
	 * @return the string
	 */
	public static String username()
	{
		return _username;
	}
	
	/**
	 *  Number of items to publish.
	 *
	 * @return the int
	 */
	public static int itemPublishCount()
	{
		return _itemPublishCount;
	}
	
	/**
	 *  Number of items common to all connections, if using multiple connections.
	 *
	 * @return the int
	 */
	public static int commonItemCount()
	{
		return _commonItemCount;
	}
	
	/**
	 *  Number of updates to send per second.
	 *
	 * @return the int
	 */
	public static int updatesPerSec()
	{
		return ProviderPerfConfig.updatesPerSec();
	}
	
	/**
	 *  Number of latency updates to send per second.
	 *
	 * @return the int
	 */
	public static int latencyUpdateRate()
	{
		return ProviderPerfConfig.latencyUpdateRate();
	}
	
    /**
     *  ID of the provided service.
     *
     * @return the int
     */
	public static int serviceId()
	{
		return ProviderPerfConfig.serviceId();
	}
	
    /**
     *  Name of the provided service.
     *
     * @return the string
     */
	public static String serviceName()
	{
		return ProviderPerfConfig.serviceName();
	}

    /**
     *  Advertised OpenLimit (set to 0 to not provide this).
     *
     * @return the int
     */
	public static int openLimit()
	{
		return ProviderPerfConfig.openLimit();
	}

    /**
     *  Converts configuration parameters to a string.
     *
     * @return the string
     */
    public static String convertToString()
	{
		return _configString;
	}

    /**
     *  Use the VA Reactor instead of the ETA Channel for sending and receiving.
     *
     * @return true, if successful
     */
    public static boolean useReactor()
    {
        return _useReactor;
    }

	/**
	 *  Provides keyfile path
	 *
	 * @return path to keyfile
	 */
	public static String keyfile()
	{
		return _keyfile;
	}

	/**
	 *  Provides key password
	 *
	 * @return key password
	 */
	public static String keypasswd()
	{
		return _keypasswd;
	}
}
