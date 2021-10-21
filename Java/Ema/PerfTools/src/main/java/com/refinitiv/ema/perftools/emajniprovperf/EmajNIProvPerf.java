package com.refinitiv.ema.perftools.emajniprovperf;

import com.refinitiv.ema.perftools.common.*;

public class EmajNIProvPerf {
    private final NIProviderPerfConfig config;
    private final XmlMsgData xmlMsgData;
    private final ProviderStats providerStats;

    private XmlItemInfoList xmlItemInfoList;

    public EmajNIProvPerf() {
        this.config = new NIProviderPerfConfig();
        this.xmlMsgData = new XmlMsgData();
        this.providerStats = new ProviderStats();
    }

    public static void main(String... args) {
        final EmajNIProvPerf emajNIProvPerf = new EmajNIProvPerf();
        emajNIProvPerf.initialize(args);
        emajNIProvPerf.run();
    }

    private void initialize(String[] args) {
        config.init(args);
        if (xmlMsgData.parseFile(config.msgFilename()) == PerfToolsReturnCodes.FAILURE) {
            System.out.printf("Failed to load message data from file '%s'.\n", config.msgFilename());
            System.exit(PerfToolsReturnCodes.FAILURE);
        }
        xmlItemInfoList = new XmlItemInfoList(config.itemPublishCount());
        if (xmlItemInfoList.parseFile(config.itemFilename()) == PerfToolsReturnCodes.FAILURE) {
            System.out.printf("Failed to load item list from file '%s'.\n", config.itemFilename());
            System.exit(-1);
        }
        this.providerStats.initialize(xmlMsgData, this.config);
    }

    private void run() {
        providerStats.run();
    }
}
