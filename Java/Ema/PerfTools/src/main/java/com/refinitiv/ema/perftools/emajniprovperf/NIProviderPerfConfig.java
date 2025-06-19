/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.emajniprovperf;

import com.refinitiv.ema.perftools.common.CommandLine;
import com.refinitiv.ema.perftools.common.BaseProviderPerfConfig;

public class NIProviderPerfConfig extends BaseProviderPerfConfig {

    private static final String DEFAULT_PERF_PROVIDER_CONFIG_PREFIX = "Perf_NIProvider_";
    private static final String DEFAULT_PERF_NIPROVIDER_SUMMARY_FILENAME = "NIProvSummary.out";
    private static final String DEFAULT_PERF_NIPROVIDER_STATS_FILENAME = "NIProvStats";

    private int itemRequestCount;       // Number of items to request. See -itemCount.
    private int commonItemCount;        // Number of items common to all providers, if using multiple connections.
    private int serviceId;              // Service ID which used by Non-Interactive provider
    private String serviceName;         // Service name which used by Non-Interactive provider
    private boolean useServiceId;       // Controls that application should use specified serviceId

    private String itemFilename;

    {
        CommandLine.addOption("latencyUpdateRate", 10, "Latency update rate per second (can specify \"all\" to send latency in every update");
        CommandLine.addOption("itemCount", 100000, "Number of items to publish non-interactively");
        CommandLine.addOption("commonItemCount", 0, "Number of items common to all providers, if using multiple connections");
        CommandLine.addOption("serviceName", "NI_PUB", "Name of the provided service");
        CommandLine.addOption("serviceId", 1, "ID of the provided service");
        CommandLine.addOption("useServiceId", false, "Flag defining whether the serviceId should be used");
        CommandLine.addOption("itemFile", "350k.xml", "Name of the file to get items from");
    }

    @Override
    protected void fillProperties() {
        super.fillProperties();
        this.itemRequestCount = CommandLine.intValue("itemCount");
        this.commonItemCount = CommandLine.intValue("commonItemCount");
        this.serviceId = CommandLine.intValue("serviceId");
        this.serviceName = CommandLine.value("serviceName");
        this.useServiceId = CommandLine.booleanValue("useServiceId");
        this.itemFilename = CommandLine.value("itemFile");

    }

    // Create config string.
    @Override
    protected void createConfigString() {
        this.configString = "--- TEST INPUTS ---\n\n" +
                "                Run Time: " + runTime + "\n" +
                "         useUserDispatch: " + (useUserDispatch ? "Yes" : "No") + "\n" +
                "                 Threads: " + threadCount + "\n" +
                "            Summary File: " + summaryFilename + "\n" +
                "    Write Stats Interval: " + writeStatsInterval + "\n" +
                "           Display Stats: " + (displayStats ? "Yes" : "No") + "\n" +
                "               Tick Rate: " + ticksPerSec + "\n" +
                "             Update Rate: " + updatesPerSec + "\n" +
                "     Latency Update Rate: " + latencyUpdateRate + "\n" +
                "      Item publish count: " + itemRequestCount + "\n" +
                "       Item common count: " + commonItemCount + "\n" +
                "               Data File: " + msgFilename + "\n" +
                "               Item File: " + itemFilename + "\n" +
                "              Service Id: " + (!useServiceId ? "<not used>" : serviceId) + "\n" +
                "            Service Name: " + serviceName;
    }

    /**
     * Item List file. Provides a list of items to open.
     *
     * @return the string
     */
    public String itemFilename() {
        return itemFilename;
    }

    /**
     * @return item count which must be published.
     */
    public int itemPublishCount() {
        return itemRequestCount;
    }

    /**
     * @return item count common for all providers.
     */
    public int commonItemCount() {
        return commonItemCount;
    }

    /**
     * @return Service ID which used by Non-Interactive provider
     */
    public int serviceId() {
        return serviceId;
    }

    /**
     * @return Service name which used by Non-Interactive provider
     */
    public String serviceName() {
        return serviceName;
    }

    /**
     * @return true if NIProvider should use specified {@link #serviceId()}. Otherwise - false.
     */
    public boolean useServiceId() {
        return useServiceId;
    }

    @Override
    protected String defaultProviderName() {
        return DEFAULT_PERF_PROVIDER_CONFIG_PREFIX;
    }

    @Override
    protected String defaultSummaryFileName() {
        return DEFAULT_PERF_NIPROVIDER_SUMMARY_FILENAME;
    }

    @Override
    protected String defaultStatsFileName() {
        return DEFAULT_PERF_NIPROVIDER_STATS_FILENAME;
    }


}
