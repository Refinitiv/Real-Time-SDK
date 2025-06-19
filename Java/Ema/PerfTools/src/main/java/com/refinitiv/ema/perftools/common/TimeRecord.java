/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

/**
 * Stores time information. This class along with {@link TimeRecordQueue}
 * are used to collect individual time differences for statistical calculation
 * in a thread-safe manner -- one thread can store information by adding to
 * the records queue and another can retrieve the information from the records
 * queue and do any desired calculation.
 */
public class TimeRecord {
    private long _startTime;    /* Recorded start time. */
    private long _endTime;        /* Recorded end time. */
    private long _ticks;        /* Units per microsecond. */

    /**
     * Recorded start time.
     *
     * @return the long
     */
    public long startTime() {
        return _startTime;
    }

    /**
     * Recorded start time.
     *
     * @param startTime the start time
     */
    public void startTime(long startTime) {
        _startTime = startTime;
    }

    /**
     * Recorded end time.
     *
     * @return the long
     */
    public long endTime() {
        return _endTime;
    }

    /**
     * Recorded end time.
     *
     * @param endTime the end time
     */
    public void endTime(long endTime) {
        _endTime = endTime;
    }

    /**
     * Units per microsecond.
     *
     * @return the long
     */
    public long ticks() {
        return _ticks;
    }

    /**
     * Units per microsecond.
     *
     * @param ticks the ticks
     */
    public void ticks(long ticks) {
        _ticks = ticks;
    }
}
