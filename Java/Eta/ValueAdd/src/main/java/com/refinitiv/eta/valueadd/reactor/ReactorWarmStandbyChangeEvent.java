/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode.*;

/**
 * An event that carries information about a warm standby channel change within the
 * {@link Reactor}. It is delivered to the application via the
 * {@link ReactorWarmStandbyChangeEventCallback} whenever the active or standby server assignment
 * changes.
 *
 * <p>The event describes:
 * <ul>
 *   <li>The warm standby mode in effect ({@link ReactorWarmStandbyMode#LOGIN_BASED} or
 *       {@link ReactorWarmStandbyMode#SERVICE_BASED}).</li>
 *   <li>The channel that is now active ({@link #currentChannel()}).</li>
 *   <li>The channel that was previously active ({@link #prevChannel()}).</li>
 *   <li>For {@link ReactorWarmStandbyMode#SERVICE_BASED} mode, the service ID and service
 *       name that triggered the change ({@link #serviceId()} / {@link #serviceName()}).</li>
 * </ul>
 *
 * @see ReactorEvent
 * @see ReactorWarmStandbyMode
 * @see ReactorWarmStandbyChannelDetails
 */
public final class ReactorWarmStandbyChangeEvent extends ReactorEvent
{
    private ReactorWarmStandbyChannelDetails prevChannel;
    private ReactorWarmStandbyChannelDetails currentChannel;
    private int warmStandbyMode = NONE;
    private int serviceId = -1;
    private String serviceName;

    private final StringBuilder stringBuilder = new StringBuilder();
    private static final String EOL = System.lineSeparator();
    private static final String TAB = "\t";

    ReactorWarmStandbyChangeEvent()
    {
    }

    /**
     * Returns the warm standby mode that is currently in effect for this event.
     *
     * <p>Possible values are:
     * <ul>
     *   <li>{@link ReactorWarmStandbyMode#LOGIN_BASED} – the active server is determined
     *       by the login connection; a failover switches the entire connection.</li>
     *   <li>{@link ReactorWarmStandbyMode#SERVICE_BASED} – the active server is determined
     *       per service; failover may affect only a subset of services.</li>
     *   <li>{@link ReactorWarmStandbyMode#NONE} – not yet set (initializer placeholder).</li>
     * </ul>
     *
     * @return one of the {@link ReactorWarmStandbyMode} integer constants
     *
     * @see ReactorWarmStandbyMode
     */
    public int warmStandbyMode()
    {
        return warmStandbyMode;
    }

    void warmStandbyMode(int warmStandbyMode)
    {
        this.warmStandbyMode = warmStandbyMode;
    }

    /**
     * Returns details about the channel that has become the new active channel.
     *
     * <p>May be {@code null} if no new active channel has been established yet (e.g., during
     * a transient failover phase).
     *
     * @return a {@link ReactorWarmStandbyChannelDetails} instance describing the new active
     *         channel, or {@code null} if unavailable
     *
     * @see ReactorWarmStandbyChannelDetails
     */
    public ReactorWarmStandbyChannelDetails currentChannel()
    {
        return currentChannel;
    }

    void currentChannel(ReactorWarmStandbyChannelDetails channel)
    {
        currentChannel = channel;
    }

    /**
     * Returns details about the channel that was the active channel before.
     *
     * <p>May be {@code null} when there was no previously active channel (e.g., on initial
     * connection establishment).
     *
     * @return a {@link ReactorWarmStandbyChannelDetails} instance describing the previously
     *         active channel, or {@code null} if unavailable
     *
     * @see ReactorWarmStandbyChannelDetails
     */
    public ReactorWarmStandbyChannelDetails prevChannel()
    {
        return prevChannel;
    }

    void prevChannel(ReactorWarmStandbyChannelDetails channel)
    {
        prevChannel = channel;
    }

    /**
     * Returns the ID of the service.
     *
     * <p>Only relevant when {@link #warmStandbyMode()} is
     * {@link ReactorWarmStandbyMode#SERVICE_BASED}. Returns {@code -1} if the service ID
     * is not applicable or has not been set.
     *
     * @return the service ID, or {@code -1} if not applicable
     */
    public int serviceId()
    {
        return serviceId;
    }

    void serviceId(int serviceId)
    {
        this.serviceId = serviceId;
    }

    /**
     * Returns the name of the service.
     *
     * <p>Only relevant when {@link #warmStandbyMode()} is
     * {@link ReactorWarmStandbyMode#SERVICE_BASED}. Returns {@code null} if the service name
     * is not applicable or has not been set.
     *
     * @return the service name, or {@code null} if not applicable
     */
    public String serviceName()
    {
        return serviceName;
    }

    void serviceName(String serviceName)
    {
        this.serviceName = serviceName;
    }

    @Override
    void clear()
    {
        super.clear();
        currentChannel = null;
        prevChannel = null;
        warmStandbyMode = NONE;
        serviceId = -1;
        serviceName = null;
    }

    @Override
    public String toString()
    {
        stringBuilder.setLength(0);

        stringBuilder.append("ReactorWarmStandbyChangeEvent: ")
                .append(EOL);

        stringBuilder.append(TAB)
                .append("warmStandbyMode: ")
                .append(warmStandbyMode() == LOGIN_BASED ? "LOGIN_BASED" :
                        warmStandbyMode() == SERVICE_BASED ? "SERVICE_BASED" : "NONE")
                .append(EOL);

        stringBuilder.append(TAB)
                .append("currentChannel: ")
                .append(currentChannel() == null ? "N/A" : currentChannel())
                .append(EOL);

        stringBuilder.append(TAB)
                .append("prevChannel: ")
                .append(prevChannel() == null ? "N/A" : prevChannel())
                .append(EOL);

        if (warmStandbyMode() == SERVICE_BASED)
        {
            stringBuilder.append(TAB)
                    .append("serviceId: ")
                    .append(serviceId())
                    .append(EOL);

            stringBuilder.append(TAB)
                    .append("serviceName: ")
                    .append(serviceName() == null ? "N/A" : serviceName())
                    .append(EOL);
        }

        return stringBuilder.toString();
    }

}
