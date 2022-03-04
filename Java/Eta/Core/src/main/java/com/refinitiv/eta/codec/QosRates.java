/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Quality of service rates enumerations convey information about the data's period of change.
 */
public class QosRates
{
    
    /**
     * Instantiates a new qos rates.
     */
    // QosRates class cannot be instantiated
    private QosRates()
    {
        throw new AssertionError();
    }

    /**
     * Qos Unspecified, indicates initialized structure and not intended to be encoded
     */
    public static final int UNSPECIFIED = 0;

    /** Qos Tick By Tick, indicates every change to information is conveyed */
    public static final int TICK_BY_TICK = 1;

    /**
     * Just In Time Conflation, indicates extreme bursts of data may be conflated
     */
    public static final int JIT_CONFLATED = 2;

    /** Time Conflated (in ms), where conflation time is provided in rateInfo */
    public static final int TIME_CONFLATED = 3;

    /**
     * Provide string representation for a Qos quality rate value.
     *
     * @param rate the rate
     * @return string representation for a Qos quality rate value
     * @see Qos
     */
    public static String toString(int rate)
    {
        switch (rate)
        {
            case QosRates.UNSPECIFIED:
                return "Unspecified";
            case QosRates.TICK_BY_TICK:
                return "TickByTick";
            case QosRates.JIT_CONFLATED:
                return "JustInTimeConflated";
            case QosRates.TIME_CONFLATED:
                return "ConflatedByRateInfo";
            default:
                return Integer.toString(rate);
        }
    }
}
