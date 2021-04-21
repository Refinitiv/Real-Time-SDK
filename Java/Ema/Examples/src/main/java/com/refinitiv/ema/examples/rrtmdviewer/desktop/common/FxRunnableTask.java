package com.refinitiv.ema.examples.rrtmdviewer.desktop.common;

import javafx.concurrent.Task;

public class FxRunnableTask extends Task<Void> {

    private final Runnable runnable;

    public FxRunnableTask(Runnable runnable) {
        this.runnable = runnable;
    }

    @Override
    protected Void call() throws Exception {
        runnable.run();
        return null;
    }
}
