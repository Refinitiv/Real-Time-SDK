/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Quality Timeliness enumerations describe the age of the data.
 */
public class QosTimeliness
{
    
    /**
     * Instantiates a new qos timeliness.
     */
    // QosTimeliness class cannot be instantiated
    private QosTimeliness()
    {
        throw new AssertionError();
    }

    /**
     * timeliness is unspecified. Typically used by QoS initialization methods
     * and not intended to be encoded or decoded.
     */
    public static final int UNSPECIFIED = 0;

    /**
     * timeliness is Real Time: data is updated as soon as new data becomes
     * available. This is the highest-quality timeliness value. Real Time in
     * conjunction with a rate of {@link QosRates#TICK_BY_TICK} is the best overall QoS.
     */
    public static final int REALTIME = 1;

    /**
     * timeliness is delayed, though the amount of delay is unknown. This is a
     * lower quality than {@link #REALTIME} and might be worse than
     * {@link #DELAYED} (in which case the delay is known).
     */
    public static final int DELAYED_UNKNOWN = 2;

    /**
     * timeliness is delayed and the amount of delay is provided in
     * {@link Qos#timeInfo()}. This is lower quality than {@link #REALTIME} and
     * might be better than {@link #DELAYED_UNKNOWN}.
     */
    public static final int DELAYED = 3;

    /**
     * Provides string representation for a Qos quality timeliness value.
     *
     * @param timeliness the timeliness
     * @return string representation for a Qos quality timeliness value
     * @see Qos
     */
    public static String toString(int timeliness)
    {
        switch (timeliness)
        {
            case QosTimeliness.UNSPECIFIED:
                return "Unspecified";
            case QosTimeliness.REALTIME:
                return "Realtime";
            case QosTimeliness.DELAYED_UNKNOWN:
                return "DelayedByUnknown";
            case QosTimeliness.DELAYED:
                return "DelayedByTimeInfo";
            default:
                return Integer.toString(timeliness);
        }
    }
}
