package com.refinitiv.ema.examples.rrtmdviewer.desktop.common.emalogging;

import java.util.logging.Handler;
import java.util.logging.LogRecord;

public class DebugHandler extends Handler {


    @Override
    public void publish(LogRecord record) {
        System.out.println(this.getFormatter().format(record) + "\n");
    }

    @Override
    public void flush() {
        System.out.flush();
    }

    @Override
    public void close() throws SecurityException {

    }
}
