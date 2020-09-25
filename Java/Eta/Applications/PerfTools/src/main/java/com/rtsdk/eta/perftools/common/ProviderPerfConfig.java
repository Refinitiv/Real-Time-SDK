package com.rtsdk.eta.perftools.common;

import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.shared.CommandLine;
import com.rtsdk.eta.transport.BindOptions;

/** Provides configuration that is not specific to any particular handler. */
public class ProviderPerfConfig
{
    private static final int ALWAYS_SEND_LATENCY_UPDATE = -1;
    private static final int ALWAYS_SEND_LATENCY_GENMSG = -1;

    private static String              _configString;

    private static int                 _ticksPerSec;                 // Controls granularity of update bursts
                                                                     // (how they must be sized to match the desired update rate).               
    private static int                 _totalBuffersPerPack;         // How many messages are packed into a given buffer.
    private static int                 _packingBufferLength;         // Size of packable buffer, if packing.
    private static int                 _refreshBurstSize;            // Number of refreshes to send in a burst(controls granularity of time-checking) 
    private static int                 _updatesPerSec;               // Total update rate per second(includes latency updates).
    private static int                 _updatesPerTick;              // Updates per tick
    private static int                 _updatesPerTickRemainder;     // Updates per tick (remainder)
    private static int                 _genMsgsPerSec;               // Total generic msg rate per second(includes latency generic msgs).
    private static int                 _genMsgsPerTick;              // Generic msgs per tick
    private static int                 _genMsgsPerTickRemainder;     // Generic msgs per tick (remainder)
    
    private static int                 _latencyUpdateRate;           // Total latency update rate per second
    private static int                 _latencyGenMsgRate;           // Total latency generic msg rate per second
    private static String			   _itemFilename;				 // Item List file. Provides a list of items to open.
    private static String              _msgFilename;                 // Data file. Describes the data to use when encoding messages.
    private static int                 _threadCount;                 // Number of provider threads to create.
    private static int                 _runTime;                     // Time application runs before exiting
    
    private static String              _serviceName;                 // Name of the provided service                
    private static int                 _serviceId;                   // ID of the provided service
    private static int                 _openLimit;                   // Advertised OpenLimit (default is set to 0 to not use this) 
    
    private static String              _portNo;                      // Port number
    private static String              _interfaceName;               // Network interface to bind to
    private static boolean             _tcpNoDelay;                  // TCP_NODELAY option for socket
    private static int                 _guaranteedOutputBuffers;     // Guaranteed Output Buffers
    private static int                 _maxOutputBuffers;            // Max Output Buffers
    private static int                 _maxFragmentSize;             // Max fragment size
    private static int                 _highWaterMark;               // High water mark
    private static int                 _sendBufSize;                 // System send buffer size
    private static int                 _recvBufSize;                 // System receive buffer size
    private static String              _summaryFilename;             // Summary file
    private static String              _statsFilename;               // Stats file
    private static String              _latencyFilename;             // Latency file
    private static boolean             _logLatencyToFile;            // Whether to log update latency information to a file
    private static int                 _writeStatsInterval;          // Controls how often statistics are written 
    private static boolean             _displayStats;                // Controls whether stats appear on the screen
    private static boolean             _directWrite;                 // direct write enabled
    private static boolean             _useReactor;                  // Use the VA Reactor instead of the ETA Channel for sending and receiving.
    private static int                 _tunnelStreamOutputBuffers;   // Tunnel Stream Guaranteed Output Buffers.
    private static boolean             _tunnelStreamBufsUsed;        // Control whether to print tunnel Stream buffers usage.

    static
    {
        CommandLine.programName("ProvPerf");
        CommandLine.addOption("p", "14002", "Port number to connect to");
        CommandLine.addOption("outputBufs", 5000, "Number of output buffers(configures guaranteedOutputBuffers in BindOptions)");
        CommandLine.addOption("maxOutputBufs", 0, "Number of max output buffers, optional(configures maxOutputBuffers in BindOptions), value is recalculated by API to match expression: 5 < \"-outputBufs\" <= \"-maxOutputBufs\"");
        CommandLine.addOption("maxFragmentSize", 6144, " Max size of buffers(configures maxFragmentSize in BindOptions)");
        CommandLine.addOption("sendBufSize", 0, "System Send Buffer Size(configures sysSendBufSize in BindOptions)");
        CommandLine.addOption("recvBufSize", 0, "System Receive Buffer Size(configures sysRecvBufSize in BindOptions)");
        CommandLine.addOption("tcpDelay", false, "Turns off tcp_nodelay in BindOptions, enabling Nagle's");
        CommandLine.addOption("highWaterMark", 0, "Sets the point that causes ETA to automatically flush");
        CommandLine.addOption("if", "", "Name of network interface to use");
        CommandLine.addOption("tickRate", 1000, "Ticks per second");
        CommandLine.addOption("updateRate", 100000, "Update rate per second");
        CommandLine.addOption("latencyUpdateRate", 10, "Latency update rate per second (can specify \"all\" to send latency in every update");
        CommandLine.addOption("genericMsgRate", 0, "Generic Msg rate per second");
        CommandLine.addOption("genericMsgLatencyRate", 0, "Latency Generic Msg rate per second (can specify \"all\" to send latency in every generic msg");
        CommandLine.addOption("maxPackCount", 1, "Maximum number of messages packed in a buffer(when count > 1, eta PackBuffer() is used");
        CommandLine.addOption("packBufSize", 6000, "If packing, sets size of buffer to use");
        CommandLine.addOption("refreshBurstSize", 10, "Number of refreshes to send in a burst(controls granularity of time-checking)");
        CommandLine.addOption("directWrite", false, "Sets direct socket write flag when using channel.write()");
        CommandLine.addOption("serviceName", "DIRECT_FEED", "Service name");
        CommandLine.addOption("serviceId", 1, "Service ID");
        CommandLine.addOption("openLimit", 1000000, "Max number of items consumer may request per connection");
        CommandLine.addOption("noDisplayStats", false, "Stop printout of stats to screen");

        CommandLine.addOption("msgFile", "MsgData.xml", "Name of the file that specifies the data content in messages");
        CommandLine.addOption("latencyFile", "ProvLatency.out", "Name of file for logging latency info");
        CommandLine.addOption("summaryFile", "ProvSummary.out", "Name of file for logging summary info");
        CommandLine.addOption("statsFile", "ProvStats", "Base name of file for logging periodic statistics");
        CommandLine.addOption("writeStatsInterval", 5, "Controls how often stats are written to the file");
        CommandLine.addOption("runTime", 360, "Runtime of the application, in seconds");
        CommandLine.addOption("threads", 1, "Number of provider threads to create");
        CommandLine.addOption("reactor", false, "Use the VA Reactor instead of the ETA Channel for sending and receiving");
        CommandLine.addOption("tunnelStreamOutputBufs", 5000, "Number of output buffers(configures guaranteedOutputBuffers in Tunnel Stream)");
        CommandLine.addOption("tunnelStreamBuffersUsed", false, "Print stats of buffers used by tunnel stream");
    }

    private ProviderPerfConfig()
    {
        
    }
    
    /**
     * Parses command-line arguments to fill in the application's configuration
     * structures.
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

    	_logLatencyToFile = false;
        _latencyFilename = CommandLine.value("latencyFile");
        _summaryFilename = CommandLine.value("summaryFile");
        _statsFilename = CommandLine.value("statsFile");
        _displayStats = !CommandLine.booleanValue("noDisplayStats");
        _interfaceName = CommandLine.value("if");
        _portNo = CommandLine.value("p");
        _tcpNoDelay = !CommandLine.booleanValue("tcpDelay");
        
        _serviceName = CommandLine.value("serviceName");
        
        // Perf Test configuration
        _msgFilename = CommandLine.value("msgFile");
        String latencyUpdateRate = CommandLine.value("latencyUpdateRate");
        String latencyGenMsgRate = CommandLine.value("genericMsgLatencyRate");
        _directWrite = CommandLine.booleanValue("directWrite");
        _useReactor = CommandLine.booleanValue("reactor");
        try
        {
            _runTime = CommandLine.intValue("runTime");
            _writeStatsInterval = CommandLine.intValue("writeStatsInterval");
            _threadCount = (CommandLine.intValue("threads") > 1 ? CommandLine.intValue("threads") : 1);            
            _guaranteedOutputBuffers = CommandLine.intValue("outputBufs");
            _maxOutputBuffers = CommandLine.intValue("maxOutputBufs");
            _maxFragmentSize = CommandLine.intValue("maxFragmentSize");
            _serviceId = CommandLine.intValue("serviceId");
            _openLimit = CommandLine.intValue("openLimit");
            _refreshBurstSize = CommandLine.intValue("refreshBurstSize");

            _updatesPerSec = CommandLine.intValue("updateRate");
            if("all".equals(latencyUpdateRate))
                _latencyUpdateRate = ALWAYS_SEND_LATENCY_UPDATE;
            else
                _latencyUpdateRate = Integer.parseInt(latencyUpdateRate);
            
            _genMsgsPerSec = CommandLine.intValue("genericMsgRate");
            if("all".equals(latencyGenMsgRate))
                _latencyGenMsgRate = ALWAYS_SEND_LATENCY_GENMSG;
            else
                _latencyGenMsgRate = Integer.parseInt(latencyGenMsgRate);
            
            _ticksPerSec = CommandLine.intValue("tickRate");
            _totalBuffersPerPack = CommandLine.intValue("maxPackCount");
            _packingBufferLength = CommandLine.intValue("packBufSize");
            _highWaterMark = CommandLine.intValue("highWaterMark");
            _sendBufSize = CommandLine.intValue("sendBufSize");
            _recvBufSize = CommandLine.intValue("recvBufSize");
            _tunnelStreamOutputBuffers = CommandLine.intValue("tunnelStreamOutputBufs");
            _tunnelStreamBufsUsed = CommandLine.booleanValue("tunnelStreamBuffersUsed");
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
        
		if (_latencyUpdateRate > _ticksPerSec)
		{
			System.err.println("Config Error: Latency Update Rate cannot be greater than tick rate.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}

		if (_latencyGenMsgRate > _ticksPerSec)
		{
			System.err.println("Config Error: Latency Generic Message Rate cannot be greater than tick rate.\n");
			System.out.println(CommandLine.optionHelpString());
			System.exit(-1);
		}
		
        if (_writeStatsInterval < 1)
        {
            System.err.println("Config error: Write Stats Interval cannot be less than 1.");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }
        
        if (_threadCount > 8)
        {
            System.err.println("Config error: Thread count cannot be greater than 8.");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }
        
        if (_totalBuffersPerPack < 1)
        {
            System.err.println("Config error: Max Pack Count cannot be less than 1.");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }
        
    	if (_totalBuffersPerPack > 1 && _packingBufferLength == 0)
    	{
    		System.err.println("Config Error: -maxPackCount set but -packBufSize is zero.\n\n");
    		System.exit(-1);
    	}

        if (_updatesPerSec != 0 && _updatesPerSec < _ticksPerSec)
        {
            System.err.println("Config Error: Update rate cannot be less than total ticks per second (unless it is zero).\n\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (_genMsgsPerSec != 0 && _genMsgsPerSec < _ticksPerSec)
        {
            System.err.println("Config Error: Generic message rate cannot be less than total ticks per second (unless it is zero).\n\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }
        
        _updatesPerTick = _updatesPerSec / _ticksPerSec;
        _updatesPerTickRemainder = _updatesPerSec % _ticksPerSec;

        _genMsgsPerTick = _genMsgsPerSec / _ticksPerSec;
        _genMsgsPerTickRemainder = _genMsgsPerSec % _ticksPerSec;
        
        createConfigString(null);
    }

    // Create config string.
    private static void createConfigString(BindOptions bindOptions)
    {
        _configString = "--- TEST INPUTS ---\n\n" +
            "          Steady State Time: " + _runTime + " sec\n" +
            "                       Port: " + _portNo + "\n" +
            "               Thread Count: " + _threadCount + "\n" +
            "             Output Buffers: " + _guaranteedOutputBuffers 
                                             + (bindOptions == null ? "" : "(effective = " + bindOptions.guaranteedOutputBuffers() + ")")
                                             + "\n" +
            "         Max Output Buffers: " + _maxOutputBuffers + ((_maxOutputBuffers > 0)? "" : "(use default)") 
                                             + (bindOptions == null ? "" : "(effective = " + bindOptions.maxOutputBuffers() + ")")
                                             + "\n" +
            "          Max Fragment Size: " + _maxFragmentSize + "\n" +
            "           Send Buffer Size: " + _sendBufSize + ((_sendBufSize > 0) ? " bytes" : "(use default)") + "\n" +
            "           Recv Buffer Size: " + _recvBufSize + ((_recvBufSize > 0) ? " bytes" : "(use default)") + "\n" +
            "             Interface Name: " + (_interfaceName.length() > 0 ? _interfaceName : "(use default)") + "\n" +
            "                Tcp_NoDelay: " + (_tcpNoDelay ? "Yes" : "No") + "\n" +
            "                  Tick Rate: " + _ticksPerSec + "\n" +
            "          Use Direct Writes: " + (_directWrite ? "Yes" : "No") + "\n" +
            "            High Water Mark: " + _highWaterMark + ((_highWaterMark > 0) ? " bytes" : "(use default)") + "\n" +
            "               Latency File: " + _latencyFilename + "\n" +
            "               Summary File: " + _summaryFilename + "\n" +
            "       Write Stats Interval: " + _writeStatsInterval + "\n" +
            "                 Stats File: " + _statsFilename + "\n" +
            "              Display Stats: " + _displayStats + "\n" + 
            "                Update Rate: " + _updatesPerSec + "\n" +
            "        Latency Update Rate: " + _latencyUpdateRate + "\n" +
            "           Generic Msg Rate: " + _genMsgsPerSec + "\n" +
            "   Generic Msg Latency Rate: " + _latencyGenMsgRate + "\n" +
            "         Refresh Burst Size: " + _refreshBurstSize + "\n" +
            "                  Data File: " + _msgFilename + "\n" +
            "                    Packing: " + (_totalBuffersPerPack <= 1 ? "No" :  "Yes(max " + _totalBuffersPerPack  + " per pack, " + _packingBufferLength +" buffer size)") + "\n" +
            "               Service Name: " + _serviceName + "\n" +
            "                 Service ID: " + _serviceId + "\n" +
            "                 Open Limit: " + _openLimit + "\n" +
            "TunnelStream Output Buffers: " + _tunnelStreamOutputBuffers + "\n" +
            "Print TunnelStream Bufs Used: " + (_tunnelStreamBufsUsed ? "Yes" : "No") + "\n" +
            "                Use Reactor: " + (_useReactor ? "Yes" : "No") + "\n";
    }

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
        return _ticksPerSec;
    }

    /**
     *  Main loop ticks per second.
     *
     * @param ticksPerSec the ticks per sec
     */
	public static void ticksPerSec(int ticksPerSec)
	{
		_ticksPerSec = ticksPerSec;
	}

    /**
     *  Number of threads that handle connections.
     *
     * @return the int
     */
    public static int threadCount()
    {
        return _threadCount;
    }
    
    /**
     *  Number of threads that handle connections.
     *
     * @param threadCount the thread count
     */
	public static void threadCount(int threadCount)
	{
		_threadCount = threadCount; 
	}

	/**
	 *  Item List file. Provides a list of items to open.
	 *
	 * @return the string
	 */
	public static String itemFilename()
	{
		return _itemFilename;
	}
	
	/**
	 *  Item List file. Provides a list of items to open.
	 *
	 * @param itemFilename the item filename
	 */
	public static void itemFilename(String itemFilename)
	{
		_itemFilename = itemFilename;
	}
	
	/**
	 *  Data file. Describes the data to use when encoding messages.
	 *
	 * @return the string
	 */
	public static String msgFilename()
	{
		return _msgFilename;
	}
	
	/**
	 *  Data file. Describes the data to use when encoding messages.
	 *
	 * @param msgFilename the msg filename
	 */
	public static void msgFilename(String msgFilename)
	{
		_msgFilename = msgFilename;
	}

	/**
	 *  Whether to log update latency information to a file.
	 *
	 * @return true, if successful
	 */
	public static boolean logLatencyToFile()
	{
	    if(_latencyFilename == null) _logLatencyToFile = false;
	    else _logLatencyToFile = (_latencyFilename.length() > 0 ? true : false);
		
		return _logLatencyToFile;
	}

	/**
	 *  Name of the latency log file.
	 *
	 * @return the string
	 */
    public static String latencyFilename()
    {
        return _latencyFilename;
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
     *  Name of the stats file.
     *
     * @return the string
     */
    public static String statsFilename()
    {
        return _statsFilename;
    }
    
	/**
	 * Stats filename.
	 *
	 * @param statsFilename the stats filename
	 */
	public static void statsFilename(String statsFilename)
	{
		_statsFilename = statsFilename;
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
     *  direct write enabled.
     *
     * @return true, if successful
     */
    public static boolean directWrite()
    {
        return _directWrite;
    }

    /**
     *  direct write enabled.
     *
     * @param directWrite the direct write
     */
	public static void directWrite(boolean directWrite)
	{
		_directWrite = directWrite;
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
     *  Guaranteed Output Buffers.
     *
     * @return the int
     */
    public static int guaranteedOutputBuffers()
    {
        return _guaranteedOutputBuffers;
    }
    
    /**
     *  Tunnel Stream Guaranteed Output Buffers.
     *
     * @return the int
     */
    public static int tunnelStreamGuaranteedOutputBuffers()
    {
        return _tunnelStreamOutputBuffers;
    }
    
    /**
     *  Max Output Buffers.
     *
     * @return the int
     */
    public static int maxOutputBuffers()
    {
        return _maxOutputBuffers;
    }

    /**
     *  Max Fragment Size.
     *
     * @return the int
     */
    public static int maxFragmentSize()
    {
        return _maxFragmentSize;
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
     *  Sets the point that causes ETA to automatically flush.
     *
     * @return the int
     */
    public static int highWaterMark()
    {
        return _highWaterMark;
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
     * Name of the provided service.
     *
     * @return the string
     */
    public static String serviceName()
    {
        return _serviceName;
    }

    /**
     * Name of the provided service.
     *
     * @param serviceName the service name
     */
	public static void serviceName(String serviceName)
	{
		_serviceName = serviceName;
	}
    
    /**
     *  ID of the provided service.
     *
     * @return the int
     */
    public static int serviceId()
    {
        return _serviceId;
    }

    /**
     *  ID of the provided service.
     *
     * @param serviceId the service id
     */
	public static void serviceId(int serviceId)
	{
		_serviceId = serviceId;
	}
    
    /**
     *  Advertised OpenLimit (set to 0 to not provide this).
     *
     * @return the int
     */
    public static int openLimit()
    {
        return _openLimit;
    }

    /**
     *  Advertised OpenLimit (set to 0 to not provide this).
     *
     * @param openLimit the open limit
     */
	public static void openLimit(int openLimit)
	{
		_openLimit = openLimit;
	}

    /**
     *  number of packing buffers.
     *
     * @return the int
     */
    public static int totalBuffersPerPack()
    {
        return _totalBuffersPerPack;
    }
    
    /**
     *  number of packing buffers.
     *
     * @param totalBuffersPerPack the total buffers per pack
     */
	public static void totalBuffersPerPack(int totalBuffersPerPack)
	{
		_totalBuffersPerPack = totalBuffersPerPack;
	}

	/**
	 *  packing buffers size.
	 *
	 * @return the int
	 */
    public static int packingBufferLength()
    {
        return _packingBufferLength;
    }

    /**
     *  packing buffers size.
     *
     * @param packingBufferLength the packing buffer length
     */
	public static void packingBufferLength(int packingBufferLength)
	{
		_packingBufferLength = packingBufferLength;
	}
    
    /**
     *  updates per second.
     *
     * @return the int
     */
    public static int updatesPerSec()
    {
        return _updatesPerSec;
    }

    /**
     *  updates per second.
     *
     * @param updatesPerSec the updates per sec
     */
	public static void updatesPerSec(int updatesPerSec)
	{
		_updatesPerSec = updatesPerSec;
	}

    /**
     *  updates per second.
     *
     * @return the int
     */
    public static int genMsgsPerSec()
    {
        return _genMsgsPerSec;
    }

    /**
     *  generic msgs per second.
     *
     * @param genMsgsPerSec the gen msgs per sec
     */
	public static void genMsgsPerSec(int genMsgsPerSec)
	{
		_genMsgsPerSec = genMsgsPerSec;
	}

	/**
	 * Number of refreshes to send in a burst(controls granularity of time-checking).
	 *
	 * @return the int
	 */
    public static int refreshBurstSize()
    {
        return _refreshBurstSize;
    }

    /**
     * Number of refreshes to send in a burst(controls granularity of time-checking).
     *
     * @param refreshBurstSize the refresh burst size
     */
	public static void refreshBurstSize(int refreshBurstSize)
	{
		_refreshBurstSize = refreshBurstSize;
	}
  
	/**
	 * Latency updates per second.
	 *
	 * @return the int
	 */
    public static int latencyUpdateRate()
    {
        return _latencyUpdateRate;
    }

    /**
     * Latency updates per second.
     *
     * @param latencyUpdateRate the latency update rate
     */
	public static void latencyUpdateRate(int latencyUpdateRate)
	{
		_latencyUpdateRate = latencyUpdateRate;
	}

	/**
	 * Latency generic msgs per second.
	 *
	 * @return the int
	 */
    public static int latencyGenMsgRate()
    {
        return _latencyGenMsgRate;
    }

    /**
     * Latency generic msgs per second.
     *
     * @param latencyGenMsgRate the latency gen msg rate
     */
	public static void latencyGenMsgRate(int latencyGenMsgRate)
	{
		_latencyGenMsgRate = latencyGenMsgRate;
	}
	
    /**
     *  Converts configuration parameters to a string.
     *
     * @return the string
     */
    public static String convertToString()
    {
        return convertToString(null);
    }

    /**
     *  Converts configuration parameters to a string with effective bindOptions values.
     *
     * @param bindOptions the bind options to convert to string
     *
     * @return the string
     */
    public static String convertToString(BindOptions bindOptions)
    {
    	if (bindOptions != null) {
    		createConfigString(bindOptions);
    	}
        return _configString;
    }

    /**
     * Returns calculated Updates per tick.
     * 
     * @return Updates to send per tick
     */
	public static int updatesPerTick()
	{
		return _updatesPerTick;
	}
	
	/**
	 * Sets Updates per tick.
	 *
	 * @param updatesPerTick the updates per tick
	 */
	public static void updatesPerTick(int updatesPerTick)
	{
		_updatesPerTick = updatesPerTick;
	}
	
	/**
     * Returns remainder updates per tick. 
     * 
     * @return Updates to send per tick remainder
     */
	public static int updatesPerTickRemainder()
	{
		return _updatesPerTickRemainder;
	}

	/**
	 * Sets remainder updates per tick. 
	 *
	 * @param updatesPerTickRemainder the updates per tick remainder
	 */
	public static void updatesPerTickRemainder(int updatesPerTickRemainder)
	{
		_updatesPerTickRemainder = updatesPerTickRemainder;
	}

    /**
     * Returns calculated Generic msgs per tick.
     * 
     * @return generic msgs to send per tick
     */
	public static int genMsgsPerTick()
	{
		return _genMsgsPerTick;
	}
	
	/**
	 * Sets Generic msgs per tick.
	 *
	 * @param genMsgsPerTick the gen msgs per tick
	 */
	public static void genMsgsPerTick(int genMsgsPerTick)
	{
		_genMsgsPerTick = genMsgsPerTick;
	}
	
	/**
     * Returns remainder generic msgs per tick. 
     * 
     * @return generic msgs to send per tick remainder
     */
	public static int genMsgsPerTickRemainder()
	{
		return _genMsgsPerTickRemainder;
	}

	/**
	 * Sets remainder generic msgs per tick. 
	 *
	 * @param genMsgsPerTickRemainder the gen msgs per tick remainder
	 */
	public static void genMsgsPerTickRemainder(int genMsgsPerTickRemainder)
	{
		_genMsgsPerTickRemainder = genMsgsPerTickRemainder;
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
	 *  Control to print TunnelStream Usage Buffers.
	 *
	 * @return the boolean
	 */
	public static boolean tunnelStreamBufsUsed()
	{
		return _tunnelStreamBufsUsed;
	}
}
