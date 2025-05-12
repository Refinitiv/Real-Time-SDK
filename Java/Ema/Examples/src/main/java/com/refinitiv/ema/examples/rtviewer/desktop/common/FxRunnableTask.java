/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.common;

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
