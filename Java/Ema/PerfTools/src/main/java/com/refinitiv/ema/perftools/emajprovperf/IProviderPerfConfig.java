/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.emajprovperf;

import com.refinitiv.ema.perftools.common.BaseProviderPerfConfig;
import com.refinitiv.ema.perftools.common.CommandLine;

public class IProviderPerfConfig extends BaseProviderPerfConfig {
    public static final String APPLICATION_NAME = "EMAProvPerf";
    public static final String APPLICATION_ID = "256";
    private static final String DEFAULT_PERF_PROVIDER_CONFIG_PREFIX = "Perf_Provider_";
    private static final String DEFAULT_PERF_IPROVIDER_SUMMARY_FILENAME = "IProvSummary.out";
    private static final String DEFAULT_PERF_IPROVIDER_STATS_FILENAME = "IProvStats";

    private int genMsgsPerTick;              // Total generic rate per tick
    private int genMsgsPerTickRemainder;     // gen msgs per tick remainder
    private int genMsgsPerSec;               // Total generic msg rate per second(includes latency generic msgs).
    private int latencyGenMsgRate;           // Total latency generic msg rate per second

    private String latencyFilename;          // Latency file
    private boolean logLatencyToFile;

    {
        CommandLine.addOption("latencyUpdateRate", 10, "Latency update rate per second (can specify \"all\" to send latency in every update");
        CommandLine.addOption("genericMsgRate", 0, "Generic Msg rate per second");
        CommandLine.addOption("genericMsgLatencyRate", 0, "Latency Generic Msg rate per second (can specify \"all\" to send latency in every generic msg");

        CommandLine.addOption("latencyFile", "IProvLatency.out", "name of file for logging latency info.");
    }

    @Override
    protected void fillProperties() {
        super.fillProperties();
        summaryFilename = CommandLine.value("summaryFile");
        statsFilename = CommandLine.value("statsFile");
        latencyFilename = CommandLine.value("latencyFile");
        logLatencyToFile = latencyFilename != null && !latencyFilename.isEmpty();
        genMsgsPerSec = CommandLine.intValue("genericMsgRate");

        final String latencyGenMsgRate = CommandLine.value("genericMsgLatencyRate");

        if ("all".equals(latencyGenMsgRate)) {
            this.latencyGenMsgRate = ALWAYS_SEND_LATENCY_GENMSG;
        } else if (latencyGenMsgRate != null) {
            this.latencyGenMsgRate = Integer.parseInt(latencyGenMsgRate);
        }

        if (this.latencyGenMsgRate > ticksPerSec) {
            System.err.println("Config Error: Latency Generic Message Rate cannot be greater than tick rate.\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (genMsgsPerSec != 0 && genMsgsPerSec < ticksPerSec) {
            System.err.println("Config Error: Generic message rate cannot be less than total ticks per second (unless it is zero).\n\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        genMsgsPerTick = genMsgsPerSec / ticksPerSec;
        genMsgsPerTickRemainder = genMsgsPerSec % ticksPerSec;
    }

    @Override
    protected void createConfigString() {
        this.configString = "--- TEST INPUTS ---\n\n" +
                "                Run Time: " + runTime + "\n" +
                "         useUserDispatch: " + (useUserDispatch ? "Yes" : "No") + "\n" +
                "                 Threads: " + threadCount + "\n" +
                "            Summary File: " + summaryFilename + "\n" +
                "        Latency Log File: " + ((latencyFilename == null || latencyFilename.isEmpty()) ? "(none)" : latencyFilename + "\n") +
                "    Write Stats Interval: " + writeStatsInterval + "\n" +
                "           Display Stats: " + (displayStats ? "Yes" : "No") + "\n" +
                "               Tick Rate: " + ticksPerSec + "\n" +
                "             Update Rate: " + updatesPerSec + "\n" +
                "     Latency Update Rate: " + latencyUpdateRate + "\n" +
                "        Generic Msg Rate: " + genMsgsPerSec + "\n" +
                "Latency Generic Msg Rate: " + latencyGenMsgRate + "\n" +
                "      Refresh Burst Size: " + refreshBurstSize + "\n" +
                "               Data File: " + msgFilename + "\n";
    }

    @Override
    protected String defaultProviderName() {
        return DEFAULT_PERF_PROVIDER_CONFIG_PREFIX;
    }

    @Override
    protected String defaultSummaryFileName() {
        return DEFAULT_PERF_IPROVIDER_SUMMARY_FILENAME;
    }

    @Override
    protected String defaultStatsFileName() {
        return DEFAULT_PERF_IPROVIDER_STATS_FILENAME;
    }

    /**
     * Whether to log update latency information to a file.
     *
     * @return true, if successful
     */
    public boolean logLatencyToFile() {
        logLatencyToFile = latencyFilename != null && latencyFilename.length() > 0;
        return logLatencyToFile;
    }

    /**
     * Name of the latency log file.
     *
     * @return the string
     */
    public String latencyFilename() {
        return latencyFilename;
    }

    /**
     * updates per second.
     *
     * @return the int
     */
    public int genMsgsPerSec() {
        return genMsgsPerSec;
    }

    /**
     * generic msgs per second.
     *
     * @param genMsgsPerSec the gen msgs per sec
     */
    public void genMsgsPerSec(int genMsgsPerSec) {
        this.genMsgsPerSec = genMsgsPerSec;
    }

    /**
     * Latency generic msgs per second.
     *
     * @return the int
     */
    public int latencyGenMsgRate() {
        return latencyGenMsgRate;
    }

    /**
     * Latency generic msgs per second.
     *
     * @param latencyGenMsgRate the latency gen msg rate
     */
    public void latencyGenMsgRate(int latencyGenMsgRate) {
        this.latencyGenMsgRate = latencyGenMsgRate;
    }

    /**
     * Returns calculated Generic msgs per tick.
     *
     * @return generic msgs to send per tick
     */
    public int genMsgsPerTick() {
        return genMsgsPerTick;
    }

    /**
     * Sets Generic msgs per tick.
     *
     * @param genMsgsPerTick the gen msgs per tick
     */
    public void genMsgsPerTick(int genMsgsPerTick) {
        this.genMsgsPerTick = genMsgsPerTick;
    }

    /**
     * Returns remainder generic msgs per tick.
     *
     * @return generic msgs to send per tick remainder
     */
    public int genMsgsPerTickRemainder() {
        return genMsgsPerTickRemainder;
    }

    /**
     * Sets remainder generic msgs per tick.
     *
     * @param genMsgsPerTickRemainder the gen msgs per tick remainder
     */
    public void genMsgsPerTickRemainder(int genMsgsPerTickRemainder) {
        this.genMsgsPerTickRemainder = genMsgsPerTickRemainder;
    }
}
