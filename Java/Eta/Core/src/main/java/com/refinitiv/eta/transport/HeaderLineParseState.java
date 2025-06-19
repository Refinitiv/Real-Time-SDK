/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.util.Objects;

enum HeaderLineParseState {
    CONTINUE(0, false),
    CONTINUE_WITH_TRANSITION(1, false),
    END_OF_LINE_LF_ONLY(1, true),
    END_OF_LINE(2, true);

    private int incrementalValue;
    private boolean lineEnd;

    HeaderLineParseState(int incrementalValue, boolean lineEnd) {
        this.incrementalValue = incrementalValue;
        this.lineEnd = lineEnd;
    }

    public int getIncrementalValue() {
        return incrementalValue;
    }

    public boolean isLineEnd() {
        return lineEnd;
    }

    public static HeaderLineParseState getEndState(HeaderLineParseState previousState) {
        return Objects.equals(CONTINUE, previousState) ? END_OF_LINE_LF_ONLY : END_OF_LINE;
    }
}
