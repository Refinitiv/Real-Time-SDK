package com.refinitiv.eta.transport;

class ParserReturnCodes {
    private ParserReturnCodes()
    {
        throw new AssertionError();
    }

    public static final int INVALID_HEADER_GROUP = -5;
    public static final int INVALID_COOKIES = -4;
    public static final int INVALID_CONNECTION_LINE = -3;
    public static final int HEADERS_NOT_PRESENTED = -2;
    public static final int FAILURE = -1;
    public static final int SUCCESS = 0;
}
