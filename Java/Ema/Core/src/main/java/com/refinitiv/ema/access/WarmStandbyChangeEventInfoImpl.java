/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyChangeEvent;

import static com.refinitiv.ema.access.WarmStandbyChannelInformation.*;

/**
 * EMA implementation of {@link WarmStandbyChangeEventInfo}.
 * <p>
 * Instances are created internally from ETA {@link ReactorWarmStandbyChangeEvent} notifications and
 * provide a snapshot of information associated with a warm standby change event.
 * </p>
 */
final class WarmStandbyChangeEventInfoImpl implements WarmStandbyChangeEventInfo
{
    private int warmStandbyMode;
    private int serviceId;
    private String previousChannelName;
    private String currentChannelName;
    private String serviceName;
    private String wsbGroupName;
    private String sessionChannelName;

    private final StringBuilder sb = new StringBuilder();
    private static final String EOL = System.lineSeparator();
    private static final String TAB = "\t";

    /**
     * Creates a new snapshot.
     * <p>
     * This is an internal value object; applications obtain instances via callbacks.
     * </p>
     */
    private WarmStandbyChangeEventInfoImpl()
    {
        warmStandbyMode = WarmStandbyMode.NONE;
        serviceId = -1;
    }

    /**
     * Converts an ETA warm standby change event to an EMA {@link WarmStandbyChangeEventInfo} snapshot.
     * <p>
     * Channel-related fields are populated when the Reactor channel's userSpecObject is a
     * {@link ChannelInfo}. Any unavailable values are set to {@code null}.
     * </p>
     *
     * @param event the ETA warm standby change event (must not be {@code null})
     * @param sessionChannelName the session channel name for request routing
     * @return a new {@link WarmStandbyChangeEventInfo} instance
     * @throws OmmInvalidUsageException if ReactorWarmStandbyChangeEvent parameter is null
     */
    static WarmStandbyChangeEventInfo create(ReactorWarmStandbyChangeEvent event, String sessionChannelName)
    {
        if (event == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("The event cannot be null.",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        WarmStandbyChangeEventInfoImpl warmStandbyChangeEventInfo = new WarmStandbyChangeEventInfoImpl();
        warmStandbyChangeEventInfo.update(event, sessionChannelName);

        return warmStandbyChangeEventInfo;
    }

    /**
     * Update an EMA {@link WarmStandbyChangeEventInfo} snapshot with an ETA event.
     * <p>
     * Channel-related fields are populated when the Reactor channel's userSpecObject is a
     * {@link ChannelInfo}. Any unavailable values are set to {@code null}.
     * </p>
     *
     * @param event the ETA warm standby change event (must not be {@code null})
     * @param sessionChannelName the session channel name for request routing
     * @throws OmmInvalidUsageException if ReactorWarmStandbyChangeEvent parameter is null
     */
    void update(ReactorWarmStandbyChangeEvent event, String sessionChannelName)
    {
        if (event == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("The event cannot be null.",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        String currentChannelName = null;
        String wsbGroupName = null;
        if (event.currentChannel() != null && event.currentChannel().userSpecObject() instanceof ChannelInfo)
        {
            ChannelInfo channelInfo = (ChannelInfo) event.currentChannel().userSpecObject();
            currentChannelName = channelInfo.name();
            ChannelInfo parentChannelInfo = channelInfo.getParentChannel();
            if (parentChannelInfo != null)
            {
                wsbGroupName = parentChannelInfo.name();
            }
        }

        String previousChannelName = null;
        if (event.prevChannel() != null && event.prevChannel().userSpecObject() instanceof ChannelInfo)
        {
            ChannelInfo channelInfo = (ChannelInfo) event.prevChannel().userSpecObject();
            previousChannelName = channelInfo.name();
        }

        this.warmStandbyMode = event.warmStandbyMode();
        this.previousChannelName = previousChannelName;
        this.currentChannelName = currentChannelName;
        if (this.warmStandbyMode == WarmStandbyMode.SERVICE_BASED)
        {
            this.serviceName = event.serviceName();
            this.serviceId = event.serviceId();
        }
        else
        {
            this.serviceName = null;
            this.serviceId = -1;
        }
        this.wsbGroupName = wsbGroupName;
        this.sessionChannelName = sessionChannelName;
    }

    @Override
    public int warmStandbyMode()
    {
        return warmStandbyMode;
    }

    @Override
    public String previousChannelName()
    {
        return previousChannelName;
    }

    @Override
    public String currentChannelName()
    {
        return currentChannelName;
    }

    @Override
    public String serviceName()
    {
        return serviceName;
    }

    @Override
    public int serviceId()
    {
        return serviceId;
    }

    @Override
    public String wsbGroupName()
    {
        return wsbGroupName;
    }

    @Override
    public String sessionChannelName()
    {
        return sessionChannelName;
    }

    @Override
    public String toString()
    {
        sb.setLength(0);

        sb.append(getClass().getSimpleName())
                .append(':')
                .append(EOL);

        sb.append(TAB)
                .append("warmStandbyMode: ")
                .append(WarmStandbyMode.toString(warmStandbyMode()))
                .append(EOL);

        if (warmStandbyMode() == WarmStandbyMode.SERVICE_BASED)
        {
                sb.append(TAB)
                        .append("serviceId: ")
                        .append(serviceId())
                        .append(EOL);

                sb.append(TAB)
                        .append("serviceName: '")
                        .append(serviceName() == null || serviceName().isBlank() ? "N/A" : serviceName())
                        .append('\'')
                        .append(EOL);
        }

        sb.append(TAB)
                .append("previousChannelName: '")
                .append(previousChannelName() == null || previousChannelName().isBlank() ? "N/A" : previousChannelName())
                .append('\'')
                .append(EOL);

        sb.append(TAB)
                .append("currentChannelName: '")
                .append(currentChannelName() == null || currentChannelName().isBlank() ? "N/A" : currentChannelName())
                .append('\'')
                .append(EOL);

        sb.append(TAB)
                .append("wsbGroupName: '")
                .append(wsbGroupName() == null || wsbGroupName().isBlank() ? "N/A" : wsbGroupName())
                .append('\'')
                .append(EOL);

        sb.append(TAB)
                .append("sessionChannelName: '")
                .append(sessionChannelName() == null || sessionChannelName().isBlank() ? "N/A" : sessionChannelName())
                .append('\'')
                .append(EOL);

        return sb.toString();
    }
}
