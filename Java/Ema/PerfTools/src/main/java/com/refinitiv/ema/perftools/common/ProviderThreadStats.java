/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022,2024 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

public class ProviderThreadStats {

    public static final long NOT_DEFINED = -1;
    private final CountStat requestCount;
    private final CountStat refreshCount;
    private final CountStat itemRefreshCount;
    private final CountStat updateCount;
    private final CountStat closeCount;
    private final CountStat postCount;
    private final CountStat statusCount;
    private final CountStat msgSentCount;
    private final CountStat outOfBuffersCount;
    private final CountStat updatePackedMsgCount;
    private final TimeRecordQueue genMsgLatencyRecords;
    private long inactiveTime;
    private long firstGenMsgSentTime;
    private long firstGenMsgRecvTime;
    private CountStat genMsgSentCount;                      /* Number of generic msgs sent. */
    private CountStat genMsgRecvCount;                      /* Number of generic msgs received. */
    private CountStat latencyGenMsgSentCount;               /* Number of latency generic msgs sent. */
    private ValueStatistics intervalGenMsgLatencyStats;     /* Generic msg latency statistics (recorded by stats thread). */
    private ValueStatistics genMsgLatencyStats;             /* Generic msg latency statistics. */

    {
        this.firstGenMsgRecvTime = NOT_DEFINED;
        this.firstGenMsgSentTime = NOT_DEFINED;

        genMsgSentCount = new CountStat();
        genMsgRecvCount = new CountStat();
        latencyGenMsgSentCount = new CountStat();
        intervalGenMsgLatencyStats = new ValueStatistics();
        genMsgLatencyStats = new ValueStatistics();
        intervalGenMsgLatencyStats.clear();
        genMsgLatencyStats.clear();

        this.requestCount = new CountStat();
        this.refreshCount = new CountStat();
        this.itemRefreshCount = new CountStat();
        this.updateCount = new CountStat();
        this.closeCount = new CountStat();
        this.postCount = new CountStat();
        this.statusCount = new CountStat();
        this.msgSentCount = new CountStat();
        this.outOfBuffersCount = new CountStat();
        this.genMsgLatencyRecords = new TimeRecordQueue();
        this.updatePackedMsgCount = new CountStat();
    }

    /**
     * Inactive time.
     *
     * @return the long
     */
    public long inactiveTime() {
        return inactiveTime;
    }

    /**
     * Inactive time.
     *
     * @param inactiveTime the inactive time
     */
    public void inactiveTime(long inactiveTime) {
        this.inactiveTime = inactiveTime;
    }

    /**
     * First gen msg sent time.
     *
     * @return the long
     */
    public long firstGenMsgSentTime() {
        return firstGenMsgSentTime;
    }

    /**
     * First gen msg sent time.
     *
     * @param firstGenMsgSentTime the first gen msg sent time
     */
    public void firstGenMsgSentTime(long firstGenMsgSentTime) {
        this.firstGenMsgSentTime = firstGenMsgSentTime;
    }

    /**
     * First gen msg recv time.
     *
     * @return the long
     */
    public long firstGenMsgRecvTime() {
        return firstGenMsgRecvTime;
    }

    /**
     * First gen msg recv time.
     *
     * @param firstGenMsgRecvTime the first gen msg recv time
     */
    public void firstGenMsgRecvTime(long firstGenMsgRecvTime) {
        this.firstGenMsgRecvTime = firstGenMsgRecvTime;
    }

    /**
     * Number of generic msgs sent.
     *
     * @return the count stat
     */
    public CountStat genMsgSentCount() {
        return genMsgSentCount;
    }

    /**
     * Number of generic msgs sent.
     *
     * @param genMsgSentCount the gen msg sent count
     */
    public void genMsgSentCount(CountStat genMsgSentCount) {
        this.genMsgSentCount = genMsgSentCount;
    }

    /**
     * Number of generic msgs received.
     *
     * @return the count stat
     */
    public CountStat genMsgRecvCount() {
        return genMsgRecvCount;
    }

    /**
     * Number of generic msgs received.
     *
     * @param genMsgRecvCount the gen msg recv count
     */
    public void genMsgRecvCount(CountStat genMsgRecvCount) {
        this.genMsgRecvCount = genMsgRecvCount;
    }

    /**
     * Number of latency generic msgs sent.
     *
     * @return the count stat
     */
    public CountStat latencyGenMsgSentCount() {
        return latencyGenMsgSentCount;
    }

    /**
     * Number of latency generic msgs sent.
     *
     * @param latencyGenMsgSentCount the latency gen msg sent count
     */
    public void latencyGenMsgSentCount(CountStat latencyGenMsgSentCount) {
        this.latencyGenMsgSentCount = latencyGenMsgSentCount;
    }

    /**
     * Generic msg latency statistics (recorded by stats thread).
     *
     * @return the value statistics
     */
    public ValueStatistics intervalGenMsgLatencyStats() {
        return intervalGenMsgLatencyStats;
    }

    /**
     * Generic msg latency statistics (recorded by stats thread).
     *
     * @param intervalGenMsgLatencyStats the interval gen msg latency stats
     */
    public void intervalGenMsgLatencyStats(ValueStatistics intervalGenMsgLatencyStats) {
        this.intervalGenMsgLatencyStats = intervalGenMsgLatencyStats;
    }

    /**
     * Generic msg latency statistics.
     *
     * @return the value statistics
     */
    public ValueStatistics genMsgLatencyStats() {
        return genMsgLatencyStats;
    }

    /**
     * Generic msg latency statistics.
     *
     * @param genMsgLatencyStats the gen msg latency stats
     */
    public void genMsgLatencyStats(ValueStatistics genMsgLatencyStats) {
        this.genMsgLatencyStats = genMsgLatencyStats;
    }

    public CountStat requestCount() {
        return requestCount;
    }

    public CountStat refreshCount() {
        return refreshCount;
    }
    
    public CountStat itemRefreshCount() {
        return itemRefreshCount;
    }

    public CountStat updateCount() {
        return updateCount;
    }

    public CountStat closeCount() {
        return closeCount;
    }

    public CountStat postCount() {
        return postCount;
    }

    public CountStat statusCount() {
        return statusCount;
    }

    public CountStat msgSentCount() {
        return msgSentCount;
    }

    public CountStat outOfBuffersCount() {
        return outOfBuffersCount;
    }

    public TimeRecordQueue genMsgLatencyRecords() {
        return genMsgLatencyRecords;
    }
    
    public CountStat updatePackedMsgCount() {
        return updatePackedMsgCount;
    }

    /**
     * Submit a time record.
     *
     * @param startTime the start time
     * @param endTime   the end time
     * @param ticks     the ticks
     */
    public void timeRecordSubmit(long startTime, long endTime, long ticks) {
        TimeRecord record;

        record = genMsgLatencyRecords.pool().poll();
        if (record == null) {
            record = new TimeRecord();
        }

        record.ticks(ticks);
        record.startTime(startTime);
        record.endTime(endTime);

        genMsgLatencyRecords.records().add(record);
    }
}
