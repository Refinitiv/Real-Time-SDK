package com.refinitiv.eta.perftools.transportperf;

/** Provides the global configuration for TransportThreads. */
class TransportThreadConfig
{
	final static int ALWAYS_SEND_LATENCY_MSG = -1;
	
	private static int		_ticksPerSec;			/* Controls granularity of msg bursts
	 												 * (how they must be sized to match the desired msg rate) */
	private static int		_totalBuffersPerPack;	/* How many messages are packed into a given buffer */
	private static int		_msgsPerSec;			/* Total msg rate per second(includes latency msgs) */
	private static int		_latencyMsgsPerSec;		/* Total latency msg rate per second */
	private static int		_msgSize;				/* Size of messages to send */
	private static int		_writeFlags;			/* Flags to use when calling Channel.write() */
	
	private static int		_msgsPerTick;			/* Messages per tick */
	private static int		_msgsPerTickRemainder;	/* Messages per tick (remainder) */
	private static boolean	_checkPings;			/* Whether ping timeouts should be monitored */
	private static String	_statsFilename;			/* Name of the statistics log file */
	private static boolean	_logLatencyToFile;		/* Whether to log latency information to a file */
	private static String	_latencyLogFilename;	/* Name of the latency log file */
	
    private TransportThreadConfig()
    {
        
    }

    /** Controls granularity of msg bursts (how they must be sized to match the desired msg rate). */
    static int ticksPerSec()
	{
		return _ticksPerSec;
	}
	
    /** Controls granularity of msg bursts (how they must be sized to match the desired msg rate). */
	static void ticksPerSec(int ticksPerSec)
	{
		_ticksPerSec = ticksPerSec;
	}
	
	/** How many messages are packed into a given buffer. */
	static int totalBuffersPerPack()
	{
		return _totalBuffersPerPack;
	}
	
	/** How many messages are packed into a given buffer. */
	static void totalBuffersPerPack(int totalBuffersPerPack)
	{
		_totalBuffersPerPack = totalBuffersPerPack;
	}
	
	/** Total msg rate per second(includes latency msgs). */
	static int msgsPerSec()
	{
		return _msgsPerSec;
	}
	
	/** Total msg rate per second(includes latency msgs). */
	static void msgsPerSec(int msgsPerSec)
	{
		_msgsPerSec = msgsPerSec;
	}
	
	/** Total latency msg rate per second. */
	static int latencyMsgsPerSec()
	{
		return _latencyMsgsPerSec;
	}
	
	/** Total latency msg rate per second. */
	static void latencyMsgsPerSec(int latencyMsgsPerSec)
	{
		_latencyMsgsPerSec = latencyMsgsPerSec;
	}
	
	/** Size of messages to send. */
	static int msgSize()
	{
		return _msgSize;
	}
	
	/** Size of messages to send. */
	static void msgSize(int msgSize)
	{
		_msgSize = msgSize;
	}
	
	/** Flags to use when calling Channel.write(). */
	static int writeFlags()
	{
		return _writeFlags;
	}
	
	/** Flags to use when calling Channel.write(). */
	static void writeFlags(int writeFlags)
	{
		_writeFlags = writeFlags;
	}
	
	/** Messages per tick. */
	static int msgsPerTick()
	{
		return _msgsPerTick;
	}
	
	/** Messages per tick. */
	static void msgsPerTick(int msgsPerTick)
	{
		_msgsPerTick = msgsPerTick;
	}
	
	/** Messages per tick (remainder). */
	static int msgsPerTickRemainder()
	{
		return _msgsPerTickRemainder;
	}
	
	/** Messages per tick (remainder). */
	static void msgsPerTickRemainder(int msgsPerTickRemainder)
	{
		_msgsPerTickRemainder = msgsPerTickRemainder;
	}

	/** Whether ping timeouts should be monitored. */
	static boolean checkPings()
	{
		return _checkPings;
	}

	/** Whether ping timeouts should be monitored. */
	static void checkPings(boolean checkPings)
	{
		_checkPings = checkPings;
	}

	/** Name of the statistics log file. */
	static String statsFilename()
	{
		return _statsFilename;
	}

	/** Name of the statistics log file. */
	static void statsFilename(String statsFilename)
	{
		_statsFilename = statsFilename;
	}
	
	/** Whether to log latency information to a file. */
	static boolean logLatencyToFile()
	{
		return _logLatencyToFile;
	}

	/** Whether to log latency information to a file. */
	static void logLatencyToFile(boolean logLatencyToFile)
	{
		_logLatencyToFile = logLatencyToFile;
	}

	/** Name of the latency log file. */
	static String latencyLogFilename()
	{
		return _latencyLogFilename;
	}

	/** Name of the latency log file. */
	static void latencyLogFilename(String latencyLogFilename)
	{
		_latencyLogFilename = latencyLogFilename;
	}
}
