/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import java.util.concurrent.TimeUnit;

public interface LoginRTT extends LoginMsg {

    public void flags(int flags);

    /**
     * The RDM Login RTT flags. Populated by {@link LoginRTTFlags}.
     *
     * @return flags
     */
    public int flags();

    /**
     * Performs a deep copy of {@link LoginRTT} object.
     *
     * @param destRTTMsg Message to copy login RTT object into. It
     *                   cannot be null.
     * @return ETA return value indicating success or failure of copy operation.
     */
    public int copy(LoginRTT destRTTMsg);

    /**
     * Sets number of retransmissions for the {@link LoginRTT} object.
     *
     * @param tcpretrans current number of retransmissions.
     */
    public void tcpRetrans(long tcpretrans);

    /**
     * Sets the RTT latency for the {@link LoginRTT} object.
     *
     * @param latency current RTT latency.
     */
    public void rtLatency(long latency);

    /**
     * Sets ticks for the {@link LoginRTT} object.
     *
     * @param ticks current ticks.
     */
    public void ticks(long ticks);

    /**
     * Gets the number of ticks in {@link LoginRTT} object.
     *
     * @return number of ticks
     */
    public long ticks();

    /**
     * Gets the rtLatency in {@link LoginRTT} object.
     *
     * @return rtLatency
     */
    public long rtLatency();

    /**
     * Gets the number of TCP retransmissions in {@link LoginRTT} object.
     *
     * @return number of TCP retransmissions
     */
    public long tcpRetrans();

    /**
     * Checks the presence of PROVIDER_DRIVEN flag
     * <p>
     * This flag can also be bulk-get by {@link #flags()}
     *
     * @return true if PROVIDER_DRIVEN is set, false otherwise
     */
    public boolean checkProviderDriven();

    /**
     * Sets PROVIDER_DRIVEN flag to true
     * <p>
     * This flag is also applied when the user calls {@link #initRTT(int)} ()} method
     */
    public void applyProviderDriven();

    /**
     * Checks the presence of HAS_TCP_RETRANS flag
     * <p>
     * This flag can also be bulk-get by {@link #flags()}
     *
     * @return true if HAS_TCP_RETRANS is set, false otherwise
     */
    public boolean checkHasTCPRetrans();

    /**
     * Sets HAS_TCP_RETRANS flag to true
     */
    public void applyHasTCPRetrans();

    /**
     * Checks the presence of ROUND_TRIP_LATENCY flag
     * <p>
     * This flag can also be bulk-get by {@link #flags()}
     *
     * @return true if ROUND_TRIP_LATENCY is set, false otherwise
     */
    public boolean checkHasRTLatency();

    /**
     * Sets ROUND_TRIP_LATENCY flag to true
     */
    public void applyHasRTLatency();

    /**
     * Initializes the {@link LoginRTTImpl} object with proper values and sets ticks.
     *
     * @param streamId unique identifier of message associated with all messages flowing within a stream.
     */
    public void initRTT(int streamId);

    /**
     * Calculates the {@link LoginRTTImpl#rtLatency()} variable for specified {@link LoginRTTImpl#ticks()}
     *
     * @param timeUnit defines the time unit of calculation result.
     * @return calculated {@link LoginRTTImpl#rtLatency()}
     */
    long calculateRTTLatency(TimeUnit timeUnit);

    /**
     * Set new time for {@link LoginRTTImpl#ticks()}
     */
    void updateRTTActualTicks();
}
