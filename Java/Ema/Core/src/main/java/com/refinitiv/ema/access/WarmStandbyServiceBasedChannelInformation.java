/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyServiceBasedChannelInfoEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyServiceBasedChannelInfoEvent.WSBPerChannelServiceInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyServiceBasedChannelInfoEvent.WSBService;


import java.util.ArrayList;
import java.util.List;

import static com.refinitiv.ema.access.EmaConfig.*;

/**
 * Represents warm standby session information for a service-based warm standby consumer channel.
 * <p>
 * EMA populates this type from the underlying ETA warm standby session state when
 * {@link OmmConsumer#getWarmStandbyChannelInformation()} or
 * {@link OmmConsumer#getWarmStandbyChannelInformation(java.util.List)} is called for a consumer
 * configured with {@link WarmStandbyMode#SERVICE_BASED}.
 * <p>
 * This class exposes the list of channels participating in the service-based warm standby group
 * and, for each channel, the services currently associated with that channel.
 *
 * @see WarmStandbyChannelInformation
 * @see WarmStandbyPerChannelServiceInfo
 * @see WarmStandbyService
 * @see OmmConsumer#getWarmStandbyChannelInformation()
 * @see OmmConsumer#getWarmStandbyChannelInformation(java.util.List)
 */
public final class WarmStandbyServiceBasedChannelInformation extends WarmStandbyChannelInformation
{
    private final List<WarmStandbyPerChannelServiceInfo> perChannelServiceList = new ArrayList<>();

    private WarmStandbyServiceBasedChannelInformation()
    {
    }

    /**
     * Returns the warm standby mode for this channel information.
     * <p>
     * Since this class represents service-based warm standby information, this method always returns
     * {@link WarmStandbyMode#SERVICE_BASED}.
     *
     * @return {@link WarmStandbyMode#SERVICE_BASED}
     */
    @Override
    public int warmStandbyMode()
    {
        return WarmStandbyMode.SERVICE_BASED;
    }

    /**
     * Returns the per-channel service information associated with this service-based session.
     * <p>
     * Each entry describes a reported warm standby channel and the list of services associated with
     * that channel. The returned list may be empty when the underlying transport
     * does not report any per-channel service information.
     *
     * @return list of per-channel service information
     *
     * @see WarmStandbyPerChannelServiceInfo
     */
    public List<WarmStandbyPerChannelServiceInfo> perChannelServiceList()
    {
        return perChannelServiceList;
    }

    /**
     * Returns a human-readable representation of the service-based warm standby session information.
     * <p>
     * The returned string includes the warm standby mode, warm standby group name, and the reported
     * per-channel service information.
     *
     * @return formatted service-based warm standby information string
     */
    @Override
    public String toString()
    {
        StringBuilder stringBuilder = super.buildStringBuilder();
        stringBuilder.insert(0, "WarmStandbyServiceBasedChannelInformation: " + EOL);

        stringBuilder.append(TAB)
                .append("perChannelServiceList: ")
                .append(EOL);
        if (perChannelServiceList.isEmpty())
        {
            stringBuilder.append(TAB)
                    .append(TAB)
                    .append("N/A")
                    .append(EOL);
        }
        else
        {
            for (WarmStandbyPerChannelServiceInfo info : perChannelServiceList())
            {
                stringBuilder.append(TAB)
                        .append(TAB)
                        .append(info == null ? "N/A" : info)
                        .append(EOL);
            }
        }
        stringBuilder.append(EOL);

        return stringBuilder.toString();
    }

    static WarmStandbyServiceBasedChannelInformation create(ReactorWarmStandbyServiceBasedChannelInfoEvent event)
    {
        if (event == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("The event cannot be null.",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        WarmStandbyServiceBasedChannelInformation serviceBasedInfo = new WarmStandbyServiceBasedChannelInformation();
        if (event.perChannelServiceList() != null && !event.perChannelServiceList().isEmpty())
        {
            for (WSBPerChannelServiceInfo wsbPerChannelServiceInfo : event.perChannelServiceList())
            {
                if (wsbPerChannelServiceInfo == null)
                {
                    continue;
                }

                WarmStandbyChannelDetails channelDetails = null;
                if(wsbPerChannelServiceInfo.channel() != null)
                {
                    channelDetails = WarmStandbyChannelDetails.create(wsbPerChannelServiceInfo.channel());
                }
                List<WarmStandbyService> serviceList = new ArrayList<>();
                if(wsbPerChannelServiceInfo.serviceList() != null && !wsbPerChannelServiceInfo.serviceList().isEmpty())
                {
                    for (WSBService wsbService : wsbPerChannelServiceInfo.serviceList())
                    {
                        if (wsbService == null)
                        {
                            continue;
                        }

                        serviceList.add(new WarmStandbyService(wsbService.serviceId(), wsbService.serviceName(),
                                wsbService.isActive()));
                    }
                }
                serviceBasedInfo.perChannelServiceList.add(new WarmStandbyPerChannelServiceInfo(channelDetails, serviceList));
            }
        }

        return serviceBasedInfo;
    }

    /**
     * Represents information for a single service reported for a warm standby channel.
     * <p>
     * Instances of this type are created by EMA from the underlying ETA warm standby event and
     * describe the service identifier, service name, and whether the service is currently active on
     * the associated channel.
     */
    public static class WarmStandbyService
    {
        private final int serviceId;
        private final String serviceName;
        private final boolean isActive;

        WarmStandbyService(int serviceId, String serviceName, boolean isActive)
        {
            this.serviceId = serviceId;
            this.serviceName = serviceName;
            this.isActive = isActive;
        }

        /**
         * Returns the service identifier associated with this warm standby service entry.
         *
         * @return service identifier
         */
        public int serviceId()
        {
            return serviceId;
        }

        /**
         * Returns the service name associated with this warm standby service entry.
         * <p>
         * The service name is copied from the underlying transport information and may be
         * {@code null} if the transport does not provide one.
         *
         * @return service name, or {@code null} if unavailable
         */
        public String serviceName()
        {
            return serviceName;
        }

        /**
         * Indicates whether this service is currently active on the associated warm standby channel.
         *
         * @return {@code true} if the service is active; otherwise {@code false}
         */
        public boolean isActive()
        {
            return isActive;
        }

        @Override
        public String toString() {
            return "WarmStandbyService{" +
                    "serviceId=" + serviceId +
                    ", serviceName='" + serviceName + '\'' +
                    ", isActive=" + isActive +
                    '}';
        }
    }

    /**
     * Represents the services reported for a single warm standby channel.
     * <p>
     * Each instance pairs optional channel connection details with the services currently associated
     * with that channel in a service-based warm standby group.
     */
    public static class WarmStandbyPerChannelServiceInfo
    {
        private final WarmStandbyChannelDetails channel;
        private final List<WarmStandbyService> serviceList;

        WarmStandbyPerChannelServiceInfo(WarmStandbyChannelDetails channel, List<WarmStandbyService> serviceList)
        {
            this.channel = channel;
            this.serviceList = serviceList;
        }

        /**
         * Returns the warm standby channel details associated with this entry.
         * <p>
         * The returned value describes the channel on which the reported services are available and
         * may be {@code null} when the underlying transport does not provide channel details.
         *
         * @return warm standby channel details, or {@code null} if unavailable
         *
         * @see WarmStandbyChannelDetails
         */
        public WarmStandbyChannelDetails channel()
        {
            return channel;
        }

        /**
         * Returns the services associated with this warm standby channel.
         * <p>
         * The returned list may be empty when no services are currently reported
         * for the channel.
         *
         * @return list of warm standby services for the channel
         *
         * @see WarmStandbyService
         */
        public List<WarmStandbyService> serviceList()
        {
            return serviceList;
        }

        @Override
        public String toString() {
            return "WarmStandbyPerChannelServiceInfo{" +
                    "channel=" + channel +
                    ", serviceList=" + serviceList +
                    '}';
        }
    }
}
