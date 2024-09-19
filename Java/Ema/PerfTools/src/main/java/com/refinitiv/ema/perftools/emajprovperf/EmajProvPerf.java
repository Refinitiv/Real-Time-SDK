/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.emajprovperf;

import com.refinitiv.ema.perftools.common.PerfToolsReturnCodes;
import com.refinitiv.ema.perftools.common.BaseProviderPerfConfig;
import com.refinitiv.ema.perftools.common.ProviderStats;
import com.refinitiv.ema.perftools.common.XmlMsgData;

public class EmajProvPerf {

    private final BaseProviderPerfConfig config;
    private final XmlMsgData xmlMsgData;
    private final ProviderStats providerStats;

    public EmajProvPerf() {
        this.config = new IProviderPerfConfig();
        this.xmlMsgData = new XmlMsgData();
        this.providerStats = new ProviderStats();
    }

    public static void main(String... args) {
        final EmajProvPerf emajProvPerf = new EmajProvPerf();
        emajProvPerf.initialize(args);
        emajProvPerf.run();
    }

    private void initialize(String[] args) {
        config.init(args);
        if (xmlMsgData.parseFile(config.msgFilename()) == PerfToolsReturnCodes.FAILURE) {
            System.out.printf("Failed to load message data from file '%s'.\n", config.msgFilename());
            System.exit(PerfToolsReturnCodes.FAILURE);
        }

        this.providerStats.initialize(xmlMsgData, this.config);
    }

    private void run() {
        providerStats.run();
    }
}
