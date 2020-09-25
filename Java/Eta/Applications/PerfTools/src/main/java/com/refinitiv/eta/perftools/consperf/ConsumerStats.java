package com.refinitiv.eta.perftools.consperf;

import com.refinitiv.eta.perftools.common.CountStat;
import com.refinitiv.eta.perftools.common.ValueStatistics;

/** Maintains counts and other values for measuring statistics on a consumer thread. */
public class ConsumerStats
{
	private long			_imageRetrievalStartTime;	/* Time at which first item request was made. */
	private long			_imageRetrievalEndTime;		/* Time at which last item refresh was received. */
	private long 			_firstUpdateTime;			/* Time at which first item update was received. */

	private CountStat		_refreshCount;				/* Number of item refreshes received. */
	private CountStat       _refreshCompleteCount;      /* Number of Refresh Complete and Data State OK */
	private CountStat		_startupUpdateCount;		/* Number of item updates received during startup. */
	private CountStat		_steadyStateUpdateCount;	/* Number of item updates received during steady state. */
	private CountStat		_requestCount;				/* Number of requests sent. */
	private CountStat		_statusCount;				/* Number of item status messages received. */
	private CountStat		_postOutOfBuffersCount;		/* Number of posts not sent due to lack of buffers. */
	private CountStat		_postSentCount;				/* Number of posts sent. */
	private CountStat		_genMsgSentCount;			/* Number of generic msgs sent. */
    private CountStat		_genMsgRecvCount;           /* Number of generic msgs received. */
    private CountStat		_latencyGenMsgSentCount;	/* Number of latency generic mesgs sent. */
    private CountStat		_genMsgOutOfBuffersCount;	/* Number of generic msgs not sent due to lack of buffers. */
	private ValueStatistics	_intervalLatencyStats;		/* Latency statistics (recorded by stats thread). */
	private ValueStatistics	_intervalPostLatencyStats;	/* Post latency statistics (recorded by stats thread). */
	private ValueStatistics	_intervalGenMsgLatencyStats;	/* Generic msg latency statistics (recorded by stats thread). */

	private ValueStatistics _startupLatencyStats;		/* Startup latency statistics. */
	private ValueStatistics _steadyStateLatencyStats;	/* Steady-state latency statistics. */
	private ValueStatistics _overallLatencyStats;		/* Overall latency statistics. */
	private ValueStatistics _postLatencyStats;			/* Posting latency statistics. */
	private ValueStatistics _genMsgLatencyStats;		/* Generic msg latency statistics. */
	private ValueStatistics _tunnelStreamBufUsageStats;	/* Tunnel Buffer Usage statistics. */
	private boolean			_imageTimeRecorded;			/* Stats thread sets this once it has recorded/printed
	 													 * this consumer's image retrieval time. */
	
	{
		_refreshCount = new CountStat();
		_refreshCompleteCount = new CountStat();
		_startupUpdateCount = new CountStat();
		_steadyStateUpdateCount = new CountStat();
		_requestCount = new CountStat();
		_statusCount = new CountStat();
		_postOutOfBuffersCount = new CountStat();
		_postSentCount = new CountStat();
		_genMsgSentCount = new CountStat();
        _genMsgRecvCount = new CountStat();
        _latencyGenMsgSentCount = new CountStat();
        _genMsgOutOfBuffersCount = new CountStat();
		_intervalLatencyStats = new ValueStatistics();
		_intervalPostLatencyStats = new ValueStatistics();
		_intervalGenMsgLatencyStats = new ValueStatistics();
		_startupLatencyStats = new ValueStatistics();
		_steadyStateLatencyStats = new ValueStatistics();
		_overallLatencyStats = new ValueStatistics();
		_postLatencyStats = new ValueStatistics();
		_genMsgLatencyStats = new ValueStatistics();
		_tunnelStreamBufUsageStats = new ValueStatistics();
	}
	
	/**
	 *  Time at which first item request was made.
	 *
	 * @return the long
	 */
	public long imageRetrievalStartTime()
	{
		return _imageRetrievalStartTime;
	}
	
	/**
	 *  Time at which first item request was made.
	 *
	 * @param imageRetrievalStartTime the image retrieval start time
	 */
	public void imageRetrievalStartTime(long imageRetrievalStartTime)
	{
		_imageRetrievalStartTime = imageRetrievalStartTime;
	}
	
	/**
	 *  Time at which last item refresh was received.
	 *
	 * @return the long
	 */
	public long imageRetrievalEndTime()
	{
		return _imageRetrievalEndTime;
	}
	
	/**
	 *  Time at which last item refresh was received.
	 *
	 * @param imageRetrievalEndTime the image retrieval end time
	 */
	public void imageRetrievalEndTime(long imageRetrievalEndTime)
	{
		_imageRetrievalEndTime = imageRetrievalEndTime;
	}

	/**
	 *  Time at which first item update was received.
	 *
	 * @return the long
	 */
	public long firstUpdateTime()
	{
		return _firstUpdateTime;
	}
	
	/**
	 *  Time at which first item update was received.
	 *
	 * @param firstUpdateTime the first update time
	 */
	public void firstUpdateTime(long firstUpdateTime)
	{
		_firstUpdateTime = firstUpdateTime;
	}

	/**
	 *  Number of item refreshes received.
	 *
	 * @return the count stat
	 */
	public CountStat refreshCount()
	{
		return _refreshCount;
	}
	
	/**
	 *  Number of item refreshes received.
	 *
	 * @param refreshCount the refresh count
	 */
	public void refreshCount(CountStat refreshCount)
	{
		_refreshCount = refreshCount;
	}
	
	/**
	 *  Number of refresh complete (with data state OK) received.
	 *
	 * @return the count stat
	 */
	public CountStat refreshCompleteCount()
	{
	    return _refreshCompleteCount;
	}
	
	/**
	 *  Number of refresh complete (with data state OK) received.
	 *
	 * @param refreshCompleteCount the refresh complete count
	 */
	public void refreshCompleteCount(CountStat refreshCompleteCount)
	{
	    _refreshCompleteCount = refreshCompleteCount;
	}

	/**
	 *  Number of item updates received during startup.
	 *
	 * @return the count stat
	 */
	public CountStat startupUpdateCount()
	{
		return _startupUpdateCount;
	}
	
	/**
	 *  Number of item updates received during startup.
	 *
	 * @param startupUpdateCount the startup update count
	 */
	public void startupUpdateCount(CountStat startupUpdateCount)
	{
		_startupUpdateCount = startupUpdateCount;
	}
	
	/**
	 *  Number of item updates received during steady state.
	 *
	 * @return the count stat
	 */
	public CountStat steadyStateUpdateCount()
	{
		return _steadyStateUpdateCount;
	}
	
	/**
	 *  Number of item updates received during steady state.
	 *
	 * @param steadyStateUpdateCount the steady state update count
	 */
	public void steadyStateUpdateCount(CountStat steadyStateUpdateCount)
	{
		_steadyStateUpdateCount = steadyStateUpdateCount;
	}

	/**
	 *  Number of requests sent.
	 *
	 * @return the count stat
	 */
	public CountStat requestCount()
	{
		return _requestCount;
	}
	
	/**
	 *  Number of requests sent.
	 *
	 * @param requestCount the request count
	 */
	public void requestCount(CountStat requestCount)
	{
		_requestCount = requestCount;
	}
	
	/**
	 *  Number of item status messages received.
	 *
	 * @return the count stat
	 */
	public CountStat statusCount()
	{
		return _statusCount;
	}

	/**
	 *  Number of item status messages received.
	 *
	 * @param statusCount the status count
	 */
	public void statusCount(CountStat statusCount)
	{
		_statusCount = statusCount;
	}
	
	/**
	 *  Number of posts not sent due to lack of buffers.
	 *
	 * @return the count stat
	 */
	public CountStat postOutOfBuffersCount()
	{
		return _postOutOfBuffersCount;
	}
	
	/**
	 *  Number of posts not sent due to lack of buffers.
	 *
	 * @param postOutOfBuffersCount the post out of buffers count
	 */
	public void postOutOfBuffersCount(CountStat postOutOfBuffersCount)
	{
		_postOutOfBuffersCount = postOutOfBuffersCount;
	}
	
	/**
	 *  Number of posts sent.
	 *
	 * @return the count stat
	 */
	public CountStat postSentCount()
	{
		return _postSentCount;
	}
	
	/**
	 *  Number of posts sent.
	 *
	 * @param postSentCount the post sent count
	 */
	public void postSentCount(CountStat postSentCount)
	{
		_postSentCount = postSentCount;
	}

	/**
	 *  Number of generic msgs not sent due to lack of buffers.
	 *
	 * @return the count stat
	 */
	public CountStat genMsgOutOfBuffersCount()
	{
		return _genMsgOutOfBuffersCount;
	}
	
	/**
	 *  Number of generic msgs not sent due to lack of buffers.
	 *
	 * @param genMsgOutOfBuffersCount the gen msg out of buffers count
	 */
	public void genMsgOutOfBuffersCount(CountStat genMsgOutOfBuffersCount)
	{
		_genMsgOutOfBuffersCount = genMsgOutOfBuffersCount;
	}
	
	/**
	 *  Number of generic msgs sent.
	 *
	 * @return the count stat
	 */
	public CountStat genMsgSentCount()
	{
		return _genMsgSentCount;
	}
	
	/**
	 *  Number of generic msgs sent.
	 *
	 * @param genMsgSentCount the gen msg sent count
	 */
	public void genMsgSentCount(CountStat genMsgSentCount)
	{
		_genMsgSentCount = genMsgSentCount;
	}
	
    /**
     *  Number of generic msgs received.
     *
     * @return the count stat
     */
    public CountStat genMsgRecvCount()
    {
        return _genMsgRecvCount;
    }
    
    /**
     *  Number of generic msgs received.
     *
     * @param genMsgRecvCount the gen msg recv count
     */
    public void genMsgRecvCount(CountStat genMsgRecvCount)
    {
        _genMsgSentCount = genMsgRecvCount;
    }
    
	/**
	 *  Number of latency generic msgs sent.
	 *
	 * @return the count stat
	 */
    public CountStat latencyGenMsgSentCount()
    {
    	return _latencyGenMsgSentCount;
    }

    /**
     *  Number of latency generic msgs sent.
     *
     * @param latencyGenMsgSentCount the latency gen msg sent count
     */
    public void latencyGenMsgSentCount(CountStat latencyGenMsgSentCount)
    {
    	_latencyGenMsgSentCount = latencyGenMsgSentCount;
    }
    
	/**
	 *  Latency statistics (recorded by stats thread).
	 *
	 * @return the value statistics
	 */
	public ValueStatistics intervalLatencyStats()
	{
		return _intervalLatencyStats;
	}
	
	/**
	 *  Latency statistics (recorded by stats thread).
	 *
	 * @param intervalLatencyStats the interval latency stats
	 */
	public void intervalLatencyStats(ValueStatistics intervalLatencyStats)
	{
		_intervalLatencyStats = intervalLatencyStats;
	}
	
	/**
	 *  Post latency statistics (recorded by stats thread).
	 *
	 * @return the value statistics
	 */
	public ValueStatistics intervalPostLatencyStats()
	{
		return _intervalPostLatencyStats;
	}
	
	/**
	 *  Post latency statistics (recorded by stats thread).
	 *
	 * @param intervalPostLatencyStats the interval post latency stats
	 */
	public void intervalPostLatencyStats(ValueStatistics intervalPostLatencyStats)
	{
		_intervalPostLatencyStats = intervalPostLatencyStats;
	}

	/**
	 *  Generic msg latency statistics (recorded by stats thread).
	 *
	 * @return the value statistics
	 */
	public ValueStatistics intervalGenMsgLatencyStats()
	{
		return _intervalGenMsgLatencyStats;
	}
	
	/**
	 *  Generic msg latency statistics (recorded by stats thread).
	 *
	 * @param intervalGenMsgLatencyStats the interval gen msg latency stats
	 */
	public void intervalGenMsgLatencyStats(ValueStatistics intervalGenMsgLatencyStats)
	{
		_intervalGenMsgLatencyStats = intervalGenMsgLatencyStats;
	}
	
	/**
	 *  Startup latency statistics.
	 *
	 * @return the value statistics
	 */
	public ValueStatistics startupLatencyStats()
	{
		return _startupLatencyStats;
	}
	
	/**
	 *  Startup latency statistics.
	 *
	 * @param startupLatencyStats the startup latency stats
	 */
	public void startupLatencyStats(ValueStatistics startupLatencyStats)
	{
		_startupLatencyStats = startupLatencyStats;
	}
	
	/**
	 *  Steady-state latency statistics.
	 *
	 * @return the value statistics
	 */
	public ValueStatistics steadyStateLatencyStats()
	{
		return _steadyStateLatencyStats;
	}
	
	/**
	 *  Steady-state latency statistics.
	 *
	 * @param steadyStateLatencyStats the steady state latency stats
	 */
	public void steadyStateLatencyStats(ValueStatistics steadyStateLatencyStats)
	{
		_steadyStateLatencyStats = steadyStateLatencyStats;
	}
	
	/**
	 *  Overall latency statistics.
	 *
	 * @return the value statistics
	 */
	public ValueStatistics overallLatencyStats()
	{
		return _overallLatencyStats;
	}
	
	/**
	 *  Overall latency statistics.
	 *
	 * @param overallLatencyStats the overall latency stats
	 */
	public void overallLatencyStats(ValueStatistics overallLatencyStats)
	{
		_overallLatencyStats = overallLatencyStats;
	}
	
	/**
	 *  Posting latency statistics.
	 *
	 * @return the value statistics
	 */
	public ValueStatistics postLatencyStats()
	{
		return _postLatencyStats;
	}
	
	/**
	 *  Posting latency statistics.
	 *
	 * @param postLatencyStats the post latency stats
	 */
	public void postLatencyStats(ValueStatistics postLatencyStats)
	{
		_postLatencyStats = postLatencyStats;
	}

	/**
	 *  Generic msg latency statistics.
	 *
	 * @return the value statistics
	 */
	public ValueStatistics genMsgLatencyStats()
	{
		return _genMsgLatencyStats;
	}
	
	/**
	 *  Tunnel stream buffer usage statistics.
	 *
	 * @param tunnelStreamBufUsageStats the tunnel stream buffer usage stats
	 */
	public void tunnelStreamBufUsageStats(ValueStatistics tunnelStreamBufUsageStats)
	{
		_tunnelStreamBufUsageStats= tunnelStreamBufUsageStats;
	}
	
	/**
	 *  Tunnel stream buffer usage statistics.
	 *
	 * @return the value statistics
	 */
	public ValueStatistics tunnelStreamBufUsageStats()
	{
		return _tunnelStreamBufUsageStats;
	}
	
	/**
	 *  Generic msg latency statistics.
	 *
	 * @param genMsgLatencyStats the gen msg latency stats
	 */
	public void genMsgLatencyStats(ValueStatistics genMsgLatencyStats)
	{
		_genMsgLatencyStats = genMsgLatencyStats;
	}
	
	/**
	 *  Stats thread sets this once it has recorded/printed
	 * this consumer's image retrieval time.
	 *
	 * @return true, if successful
	 */
	public boolean imageTimeRecorded()
	{
		return _imageTimeRecorded;
	}

	/**
	 *  Stats thread sets this once it has recorded/printed
	 * this consumer's image retrieval time.
	 *
	 * @param imageTimeRecorded the image time recorded
	 */
	public void imageTimeRecorded(boolean imageTimeRecorded)
	{
		_imageTimeRecorded = imageTimeRecorded;
	}
}
