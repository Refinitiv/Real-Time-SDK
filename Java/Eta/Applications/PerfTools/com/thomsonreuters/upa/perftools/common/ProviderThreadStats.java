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

    public long inactiveTime()
    {
    	return _inactiveTime;
    }
    
    public void inactiveTime(long inactiveTime)
    {
    	_inactiveTime = inactiveTime;
    }
    
    public long firstGenMsgSentTime()
    {
        return _firstGenMsgSentTime;
    }
    
    public void firstGenMsgSentTime(long firstGenMsgSentTime)
    {
        _firstGenMsgSentTime = firstGenMsgSentTime;
    }
    
    public long firstGenMsgRecvTime()
    {
        return _firstGenMsgRecvTime;
    }
    
    public void firstGenMsgRecvTime(long firstGenMsgRecvTime)
    {
        _firstGenMsgRecvTime = firstGenMsgRecvTime;
    }
    
    /** Number of generic msgs sent. */
    public CountStat genMsgSentCount()
    {
        return _genMsgSentCount;
    }
    
    /** Number of generic msgs sent. */
    public void genMsgSentCount(CountStat genMsgSentCount)
    {
        _genMsgSentCount = genMsgSentCount;
    }
    
    /** Number of generic msgs received. */
    public CountStat genMsgRecvCount()
    {
        return _genMsgRecvCount;
    }
    
    /** Number of generic msgs received. */
    public void genMsgRecvCount(CountStat genMsgRecvCount)
    {
        _genMsgRecvCount = genMsgRecvCount;
    }

    /** Number of latency generic msgs sent. */
    public CountStat latencyGenMsgSentCount()
    {
        return _latencyGenMsgSentCount;
    }
    
    /** Number of latency generic msgs sent. */
    public void latencyGenMsgSentCount(CountStat latencyGenMsgSentCount)
    {
        _latencyGenMsgSentCount = latencyGenMsgSentCount;
    }
    
    /** Generic msg latency statistics (recorded by stats thread). */
    public ValueStatistics intervalGenMsgLatencyStats()
    {
        return _intervalGenMsgLatencyStats;
    }
    
    /** Generic msg latency statistics (recorded by stats thread). */
    public void intervalGenMsgLatencyStats(ValueStatistics intervalGenMsgLatencyStats)
    {
        _intervalGenMsgLatencyStats = intervalGenMsgLatencyStats;
    }

    /** Generic msg latency statistics. */
    public ValueStatistics genMsgLatencyStats()
    {
        return _genMsgLatencyStats;
    }
    
    /** Generic msg latency statistics. */
    public void genMsgLatencyStats(ValueStatistics genMsgLatencyStats)
    {
        _genMsgLatencyStats = genMsgLatencyStats;
    }

    public ValueStatistics refreshBufLenStats()
    {
        return _refreshBufLenStats;
    }
    public void refreshBufLenStats(ValueStatistics refreshBufLenStats)
    {
        _refreshBufLenStats = refreshBufLenStats;
    }
    public ValueStatistics updateBufLenStats()
    {
        return _updateBufLenStats;
    }
    public void updateBufLenStats(ValueStatistics updateBufLenStats)
    {
        _updateBufLenStats = updateBufLenStats;
    }
    public ValueStatistics genMsgBufLenStats()
    {
        return _genMsgBufLenStats;
    }
    public void genMsgBufLenStats(ValueStatistics genMsgBufLenStats)
    {
        _genMsgBufLenStats = genMsgBufLenStats;
    }
}
