/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import java.io.File;
import java.io.PrintWriter;

public class LogFileInfo {
    private File file;
    private PrintWriter writer;
    private boolean supportedWriting;

    public File file() {
        return file;
    }

    public void file(File file) {
        this.file = file;
    }

    public PrintWriter writer() {
        return writer;
    }

    public void writer(PrintWriter writer) {
        this.writer = writer;
    }

    public boolean supportedWriting() {
        return supportedWriting;
    }

    public void supportedWriting(boolean supportedWriting) {
        this.supportedWriting = supportedWriting;
    }
}
