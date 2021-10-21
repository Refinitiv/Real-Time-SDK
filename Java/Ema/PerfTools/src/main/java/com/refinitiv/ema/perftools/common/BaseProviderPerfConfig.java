package com.refinitiv.ema.perftools.common;

import com.refinitiv.eta.codec.CodecReturnCodes;

import java.net.InetAddress;

public abstract class BaseProviderPerfConfig {
    public static final int ALWAYS_SEND_LATENCY_UPDATE = -1;
    public static final int ALWAYS_SEND_LATENCY_GENMSG = -1;

    protected String configString;             // configuration string which will be print after start
    protected String providerName;             // prefix of provider name in the EMAConfig.xml
    protected boolean useCustomName;           // Defines whenever provider name was specified
    protected int updatesPerTick;              // Total update rate per tick
    protected int updatesPerTickRemainder;     // updates per tick remainder
    protected int ticksPerSec;                 // Controls granularity of update bursts
    protected int updatesPerSec;               // Total update rate per second(includes latency updates).
    protected int refreshBurstSize;            // Number of refreshes to send in a burst(controls granularity of time-checking)
    protected int latencyUpdateRate;           // Total latency update rate per second
    protected String msgFilename;              // Data file. Describes the data to use when encoding messages.
    protected int threadCount;                 // Number of provider threads to create.
    protected int runTime;                     // Time application runs before exiting

    protected int writeStatsInterval;          // Controls how often statistics are written
    protected boolean displayStats;            // Controls whether stats appear on the screen\
    protected boolean useUserDispatch;         // Controls opportunity for manually dispatching of the incoming messages
    private String position;                   // provider's position

    protected String summaryFilename;          // Summary file
    protected String statsFilename;            // Stats file

    protected String keyFile;
    protected String keyPassw;

    {
        CommandLine.programName("emajProvPerf");
        CommandLine.addOption("providerName", defaultProviderName(), "Base name of provider configuration in EMAConfig.xml");
        CommandLine.addOption("refreshBurstSize", 10, "Refreshes");
        CommandLine.addOption("tickRate", 1000, "Ticks per second");
        CommandLine.addOption("updateRate", 100000, "Update rate per second");
        CommandLine.addOption("noDisplayStats", false, "Stop printout of stats to screen");
        CommandLine.addOption("threads", 1, "Amount of executable threads");
        CommandLine.addOption("runTime", 360, "Runtime of the application, in seconds");

        CommandLine.addOption("msgFile", "MsgData.xml", "Name of the file that specifies the data content in messages");
        CommandLine.addOption("writeStatsInterval", 5, "Controls how ofthen stats are written to the file.");

        CommandLine.addOption("useUserDispatch", false, "True if user wants to dispatch EMA manually");

        CommandLine.addOption("summaryFile", defaultSummaryFileName(), "Name for logging summary info");
        CommandLine.addOption("statsFile", defaultStatsFileName(), "Base name of file for logging periodic statistics");

        CommandLine.addOption("keyfile", "", "Keystore file location and name");
        CommandLine.addOption("keypasswd", "", "Keystore password");
    }

    /**
     * Parses command-line arguments to fill in the application's configuration
     * structures.
     *
     * @param args the args
     */
    public final void init(String[] args) {
        try {
            CommandLine.parseArgs(args);
        } catch (IllegalArgumentException ile) {
            System.err.println("Error loading command line arguments:\t");
            System.err.println(ile.getMessage());
            System.err.println();
            System.err.println(CommandLine.optionHelpString());
            System.exit(CodecReturnCodes.FAILURE);
        }
        fillProperties();
        createConfigString();
    }

    protected void fillProperties() {
        providerName = CommandLine.value("providerName");
        useCustomName = !defaultProviderName().equals(providerName);
        summaryFilename = CommandLine.value("summaryFile");
        statsFilename = CommandLine.value("statsFile");
        displayStats = !CommandLine.booleanValue("noDisplayStats");
        keyFile = CommandLine.value("keyfile");
        keyPassw = CommandLine.value("keypasswd");

        // Perf Test configuration
        msgFilename = CommandLine.value("msgFile");
        String latencyUpdateRate = CommandLine.value("latencyUpdateRate");
        try {
            runTime = CommandLine.intValue("runTime");
            writeStatsInterval = CommandLine.intValue("writeStatsInterval");
            threadCount = Math.max(CommandLine.intValue("threads"), 1);
            refreshBurstSize = CommandLine.intValue("refreshBurstSize");

            updatesPerSec = CommandLine.intValue("updateRate");
            if ("all".equals(latencyUpdateRate)) {
                this.latencyUpdateRate = ALWAYS_SEND_LATENCY_UPDATE;
            } else if (latencyUpdateRate != null) {
                this.latencyUpdateRate = Integer.parseInt(latencyUpdateRate);
            }

            ticksPerSec = CommandLine.intValue("tickRate");
            this.useUserDispatch = CommandLine.booleanValue("useUserDispatch");
        } catch (NumberFormatException ile) {
            System.err.println("Invalid argument, number expected.\t");
            System.err.println(ile.getMessage());
            System.err.println();
            System.err.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (ticksPerSec < 1) {
            System.err.println("Config Error: Tick rate cannot be less than 1.");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (this.latencyUpdateRate > ticksPerSec) {
            System.err.println("Config Error: Latency Update Rate cannot be greater than tick rate.\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (writeStatsInterval < 1) {
            System.err.println("Config error: Write Stats Interval cannot be less than 1.");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        if (threadCount > 8) {
            System.err.println("Config error: Thread count cannot be greater than 8.");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }


        if (updatesPerSec != 0 && updatesPerSec < ticksPerSec) {
            System.err.println("Config Error: Update rate cannot be less than total ticks per second (unless it is zero).\n\n");
            System.out.println(CommandLine.optionHelpString());
            System.exit(-1);
        }

        updatesPerTick = updatesPerSec / ticksPerSec;
        updatesPerTickRemainder = updatesPerSec % ticksPerSec;

        initPosition();
    }

    // Create config string.
    protected abstract void createConfigString();

    private void initPosition() {
        try {
            this.position = InetAddress.getLocalHost().getHostAddress() + "/"
                    + InetAddress.getLocalHost().getHostName();
        } catch (Exception e) {
            this.position = "1.1.1.1/net";
        }
    }

    /**
     * Prefix for configuration definition in the EMAConfig.xml
     *
     * @return the String
     */
    public String providerName() {
        return this.providerName;
    }

    /**
     *
     * @return true if {@link #providerName()} was specified.
     */
    public boolean useCustomName() {
        return useCustomName;
    }

    /**
     * Time application runs before exiting.
     *
     * @return the int
     */
    public int runTime() {
        return runTime;
    }

    /**
     * Main loop ticks per second.
     *
     * @return the int
     */
    public int ticksPerSec() {
        return ticksPerSec;
    }

    /**
     * Main loop ticks per second.
     *
     * @param ticksPerSec the ticks per sec
     */
    public void ticksPerSec(int ticksPerSec) {
        this.ticksPerSec = ticksPerSec;
    }

    /**
     * Number of threads that handle connections.
     *
     * @return the int
     */
    public int threadCount() {
        return threadCount;
    }

    /**
     * Number of threads that handle connections.
     *
     * @param threadCount the thread count
     */
    public void threadCount(int threadCount) {
        this.threadCount = threadCount;
    }

    /**
     * Data file. Describes the data to use when encoding messages.
     *
     * @return the string
     */
    public String msgFilename() {
        return msgFilename;
    }

    /**
     * Data file. Describes the data to use when encoding messages.
     *
     * @param msgFilename the msg filename
     */
    public void msgFilename(String msgFilename) {
        this.msgFilename = msgFilename;
    }

    /**
     * Controls how often statistics are written.
     *
     * @return the int
     */
    public int writeStatsInterval() {
        return writeStatsInterval;
    }

    /**
     * Controls whether stats appear on the screen.
     *
     * @return true, if successful
     */
    public boolean displayStats() {
        return displayStats;
    }

    /**
     * updates per second.
     *
     * @return the int
     */
    public int updatesPerSec() {
        return updatesPerSec;
    }

    /**
     * updates per second.
     *
     * @param updatesPerSec the updates per sec
     */
    public void updatesPerSec(int updatesPerSec) {
        this.updatesPerSec = updatesPerSec;
    }

    /**
     * Number of refreshes to send in a burst(controls granularity of time-checking).
     *
     * @return the int
     */
    public int refreshBurstSize() {
        return refreshBurstSize;
    }

    /**
     * Number of refreshes to send in a burst(controls granularity of time-checking).
     *
     * @param refreshBurstSize the refresh burst size
     */
    public void refreshBurstSize(int refreshBurstSize) {
        this.refreshBurstSize = refreshBurstSize;
    }

    /**
     * Latency updates per second.
     *
     * @return the int
     */
    public int latencyUpdateRate() {
        return latencyUpdateRate;
    }

    /**
     * Latency updates per second.
     *
     * @param latencyUpdateRate the latency update rate
     */
    public void latencyUpdateRate(int latencyUpdateRate) {
        this.latencyUpdateRate = latencyUpdateRate;
    }



    /**
     * Returns calculated Updates per tick.
     *
     * @return Updates to send per tick
     */
    public int updatesPerTick() {
        return updatesPerTick;
    }

    /**
     * Sets Updates per tick.
     *
     * @param updatesPerTick the updates per tick
     */
    public void updatesPerTick(int updatesPerTick) {
        this.updatesPerTick = updatesPerTick;
    }

    /**
     * Returns remainder updates per tick.
     *
     * @return Updates to send per tick remainder
     */
    public int updatesPerTickRemainder() {
        return updatesPerTickRemainder;
    }

    /**
     * Sets remainder updates per tick.
     *
     * @param updatesPerTickRemainder the updates per tick remainder
     */
    public void updatesPerTickRemainder(int updatesPerTickRemainder) {
        this.updatesPerTickRemainder = updatesPerTickRemainder;
    }

    public String configString() {
        return configString;
    }

    public void configString(String configString) {
        this.configString = configString;
    }

    public void runTime(int runTime) {
        this.runTime = runTime;
    }

    public void writeStatsInterval(int writeStatsInterval) {
        this.writeStatsInterval = writeStatsInterval;
    }

    public void displayStats(boolean displayStats) {
        this.displayStats = displayStats;
    }

    public boolean useUserDispatch() {
        return useUserDispatch;
    }

    public void useUserDispatch(boolean useUserDispatch) {
        this.useUserDispatch = useUserDispatch;
    }

    public String position() {
        return position;
    }
    /**
     * Name of the summary log file.
     *
     * @return the string
     */
    public String summaryFilename() {
        return summaryFilename;
    }

    /**
     * Name of the stats file.
     *
     * @return the string
     */
    public String statsFilename() {
        return statsFilename;
    }


    protected abstract String defaultProviderName();

    protected abstract String defaultSummaryFileName();

    protected abstract String defaultStatsFileName();

    public String keyFile() {
        return keyFile;
    }

    public void keyFile(String keyFile) {
        this.keyFile = keyFile;
    }

    public String keyPassw() {
        return keyPassw;
    }

    public void keyPassw(String keyPassw) {
        this.keyPassw = keyPassw;
    }
    /**
     * Converts configuration parameters to a string with effective bindOptions values.
     *
     * @return the string
     */
    public String toString() {
        return configString;
    }
}
