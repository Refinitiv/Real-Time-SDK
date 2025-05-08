/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.common;

import java.util.Arrays;

public class OMMViewerError {

    private final StringBuilder sb = new StringBuilder();
    private boolean isFailed;

    public boolean isFailed() {
        return isFailed;
    }

    public void setFailed(boolean value) {
        this.isFailed = value;
    }

    public void clear() {
        sb.setLength(0);
        sb.append(System.lineSeparator()); //indent from previous error.
        isFailed = false;
    }

    public void appendErrorText(String error) {
        sb.append(error).append(System.lineSeparator());
    }

    @Override
    public String toString() {
        return sb.toString();
    }

    public void appendStackTrace(Exception exception) {
        Arrays.stream(exception.getStackTrace()).map(StackTraceElement::toString).forEach(s -> {
            sb.append(s).append(System.lineSeparator());
        });
    }
}
