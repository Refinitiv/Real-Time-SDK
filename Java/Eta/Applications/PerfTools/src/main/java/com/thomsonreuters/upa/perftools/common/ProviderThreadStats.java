package com.thomsonreuters.upa.perftools.common;

/** Statistics associated with a ProviderThread. */
public class ProviderThreadStats
{
	private long			 _inactiveTime;
    private long            _firstGenMsgSentTime;
    private long            _firstGenMsgRecvTime;
    private CountStat       _genMsgSentCount;           /* Number of generic msgs sent. */
    private CountStat       _genMsgRecvCount;           /* Number of generic msgs received. */
    private CountStat       _latencyGenMsgSentCount;    /* Number of latency generic msgs sent. */
    private ValueStatistics _intervalGenMsgLatencyStats;    /* Generic msg latency statistics (recorded by stats thread). */
    private ValueStatistics _genMsgLatencyStats;        /* Generic msg latency statistics. */
   
    private ValueStatistics _refreshBufLenStats;
    private ValueStatistics _updateBufLenStats;
    private ValueStatistics _genMsgBufLenStats;
    
    {
        _firstGenMsgSentTime = 0;
        _firstGenMsgRecvTime = 0;
        _genMsgSentCount = new CountStat();
        _genMsgRecvCount = new CountStat();
        _latencyGenMsgSentCount = new CountStat();
        _intervalGenMsgLatencyStats = new ValueStatistics();
        _genMsgLatencyStats = new ValueStatistics();
        _intervalGenMsgLatencyStats.clear();
        _genMsgLatencyStats.clear();
           
        _refreshBufLenStats = new ValueStatistics();
        _updateBufLenStats = new ValueStatistics();
        _genMsgBufLenStats = new ValueStatistics();
        _refreshBufLenStats.clear();
        _updateBufLenStats.clear();
        _genMsgBufLenStats.clear();
    }

    /**
     * Inactive time.
     *
     * @return the long
     */
    public long inactiveTime()
    {
    	return _inactiveTime;
    }
    
    /**
     * Inactive time.
     *
     * @param inactiveTime the inactive time
     */
    public void inactiveTime(long inactiveTime)
    {
    	_inactiveTime = inactiveTime;
    }
    
    /**
     * First gen msg sent time.
     *
     * @return the long
     */
    public long firstGenMsgSentTime()
    {
        return _firstGenMsgSentTime;
    }
    
    /**
     * First gen msg sent time.
     *
     * @param firstGenMsgSentTime the first gen msg sent time
     */
    public void firstGenMsgSentTime(long firstGenMsgSentTime)
    {
        _firstGenMsgSentTime = firstGenMsgSentTime;
    }
    
    /**
     * First gen msg recv time.
     *
     * @return the long
     */
    public long firstGenMsgRecvTime()
    {
        return _firstGenMsgRecvTime;
    }
    
    /**
     * First gen msg recv time.
     *
     * @param firstGenMsgRecvTime the first gen msg recv time
     */
    public void firstGenMsgRecvTime(long firstGenMsgRecvTime)
    {
        _firstGenMsgRecvTime = firstGenMsgRecvTime;
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
        _genMsgRecvCount = genMsgRecvCount;
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
     *  Generic msg latency statistics.
     *
     * @return the value statistics
     */
    public ValueStatistics genMsgLatencyStats()
    {
        return _genMsgLatencyStats;
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
     * Refresh buf len stats.
     *
     * @return the value statistics
     */
    public ValueStatistics refreshBufLenStats()
    {
        return _refreshBufLenStats;
    }
    
    /**
     * Refresh buf len stats.
     *
     * @param refreshBufLenStats the refresh buf len stats
     */
    public void refreshBufLenStats(ValueStatistics refreshBufLenStats)
    {
        _refreshBufLenStats = refreshBufLenStats;
    }
    
    /**
     * Update buf len stats.
     *
     * @return the value statistics
     */
    public ValueStatistics updateBufLenStats()
    {
        return _updateBufLenStats;
    }
    
    /**
     * Update buf len stats.
     *
     * @param updateBufLenStats the update buf len stats
     */
    public void updateBufLenStats(ValueStatistics updateBufLenStats)
    {
        _updateBufLenStats = updateBufLenStats;
    }
    
    /**
     * Gen msg buf len stats.
     *
     * @return the value statistics
     */
    public ValueStatistics genMsgBufLenStats()
    {
        return _genMsgBufLenStats;
    }
    
    /**
     * Gen msg buf len stats.
     *
     * @param genMsgBufLenStats the gen msg buf len stats
     */
    public void genMsgBufLenStats(ValueStatistics genMsgBufLenStats)
    {
        _genMsgBufLenStats = genMsgBufLenStats;
    }
}
