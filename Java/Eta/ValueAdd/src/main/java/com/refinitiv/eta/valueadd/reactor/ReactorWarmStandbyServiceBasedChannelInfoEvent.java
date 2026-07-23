/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.*;

import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode.SERVICE_BASED;

/**
 * Represents the Reactor warm standby service based channel information.
 *
 * @see ReactorWarmStandbyChannelInfoEvent
 */
public final class ReactorWarmStandbyServiceBasedChannelInfoEvent extends ReactorWarmStandbyChannelInfoEvent
{
    private final List<WSBPerChannelServiceInfo> perChannelServiceList = new ArrayList<>();

    private ReactorWarmStandbyServiceBasedChannelInfoEvent()
    {
    }

    @Override
    public int warmStandbyMode()
    {
        return SERVICE_BASED;
    }

    @Override
    public void clear()
    {
        super.clear();
        perChannelServiceList.clear();
    }

    /**
     * Returns a list of per channel service information
     *
     * @return a list of WSBPerChannelServiceInfo
     *
     * @see WSBPerChannelServiceInfo
     */
    public List<WSBPerChannelServiceInfo> perChannelServiceList()
    {
        return perChannelServiceList;
    }

    void perChannelServiceList(List<WSBPerChannelServiceInfo> perChannelServiceList)
    {
        this.perChannelServiceList.clear();
        if (perChannelServiceList != null && !perChannelServiceList.isEmpty())
        {
            this.perChannelServiceList.addAll(perChannelServiceList);
        }
    }

    @Override
    public String toString()
    {
        StringBuilder stringBuilder = super.buildStringBuilder();
        stringBuilder.insert(0, "ReactorWarmStandbyServiceBasedChannelInfoEvent: " + EOL);

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
            for (WSBPerChannelServiceInfo info : perChannelServiceList())
            {
                stringBuilder.append(TAB)
                        .append(TAB)
                        .append(info)
                        .append(EOL);
            }
        }

        return stringBuilder.toString();
    }

    static ReactorWarmStandbyServiceBasedChannelInfoEvent create(ReactorChannel reactorChannel, Set<String> serviceNames)
    {
        ReactorWarmStandbyServiceBasedChannelInfoEvent wsbChannelInfoEvent =
                new ReactorWarmStandbyServiceBasedChannelInfoEvent();

        if (reactorChannel == null || reactorChannel.warmStandByHandlerImpl == null)
        {
            return wsbChannelInfoEvent;
        }

        ReactorWarmStandbyHandler warmStandbyHandler = reactorChannel.warmStandByHandlerImpl;
        ReactorWarmStandbyGroupImpl warmStandbyGroup = warmStandbyHandler.currentWarmStandbyGroupImpl();
        if (warmStandbyGroup == null)
        {
            return wsbChannelInfoEvent;
        }

        Map<ReactorChannel, List<WSBService>> mapChannelIntoServiceList = new LinkedHashMap<>();

        for (WlInteger serviceId : warmStandbyGroup._perServiceById.keySet())
        {
            ReactorWSBService service = warmStandbyGroup._perServiceById.get(serviceId);
            if (service != null)
            {
                for (ReactorChannel processReactorChannel : service.channels)
                {
                    WlService wlService = processReactorChannel.watchlist()
                            .directoryHandler()._serviceCache._servicesByIdTable.get(service.serviceId);
                    if (wlService != null)
                    {
                        String serviceName = wlService.rdmService().info().serviceName().toString();
                        if (serviceNames == null || serviceNames.isEmpty() || serviceNames.contains(serviceName))
                        {
                            // Add service info for this reactor channel
                            boolean isActive = service.activeChannel == processReactorChannel;
                            mapChannelIntoServiceList
                                    .computeIfAbsent(processReactorChannel, k -> new ArrayList<>())
                                    .add(new WSBService(serviceId.value(), serviceName, isActive));
                        }
                    }
                }
            }
        }

        List<WSBPerChannelServiceInfo> perChannelServiceList = new ArrayList<>(mapChannelIntoServiceList.size());
        mapChannelIntoServiceList
                .forEach((channel, list) -> perChannelServiceList
                        .add(new WSBPerChannelServiceInfo(ReactorWarmStandbyChannelDetails.create(channel.channel()), list)));

        wsbChannelInfoEvent.perChannelServiceList(perChannelServiceList);

        return wsbChannelInfoEvent;
    }

    /**
     * Represents a warm standby service information.
     */
    public static class WSBService
    {
        private final int serviceId;
        private final String serviceName;
        private final boolean isActive;

        WSBService(int serviceId, String serviceName, boolean isActive)
        {
            this.serviceId = serviceId;
            this.serviceName = serviceName;
            this.isActive = isActive;
        }

        /**
         * The service ID
         *
         * @return the service ID
         */
        public int serviceId()
        {
            return serviceId;
        }

        /**
         * The service name
         *
         * @return the service name
         */
        public String serviceName()
        {
            return serviceName;
        }

        /**
         * The service state
         *
         * @return true, if the service is active
         */
        public boolean isActive()
        {
            return isActive;
        }

        @Override
        public String toString() {
            return "WSBService{" +
                    "serviceId=" + serviceId +
                    ", serviceName='" + serviceName + '\'' +
                    ", isActive=" + isActive +
                    '}';
        }
    }

    /**
     * Represents a warm standby services per reactor channel information.
     */
    public static class WSBPerChannelServiceInfo
    {
        private final ReactorWarmStandbyChannelDetails channel;
        private final List<WSBService> serviceList;

        WSBPerChannelServiceInfo(ReactorWarmStandbyChannelDetails channel, List<WSBService> serviceList)
        {
            this.channel = channel;
            this.serviceList = serviceList;
        }

        /**
         * Returns a channel information
         *
         * @return ReactorWarmStandbyChannelDetails
         *
         * @see ReactorWarmStandbyChannelDetails
         */
        public ReactorWarmStandbyChannelDetails channel()
        {
            return channel;
        }

        /**
         * Returns a list of services for a channel
         *
         * @return a list of services
         *
         * @see WSBService
         */
        public List<WSBService> serviceList()
        {
            return serviceList;
        }

        @Override
        public String toString() {
            return "WSBPerChannelServiceInfo{" +
                    "channel=" + channel +
                    ", serviceList=" + serviceList +
                    '}';
        }
    }
}
