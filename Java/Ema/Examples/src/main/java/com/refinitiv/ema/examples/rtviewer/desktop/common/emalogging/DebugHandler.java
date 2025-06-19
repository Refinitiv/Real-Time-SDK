/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.common.emalogging;

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
