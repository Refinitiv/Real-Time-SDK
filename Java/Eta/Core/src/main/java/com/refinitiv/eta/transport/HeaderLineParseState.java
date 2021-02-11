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
