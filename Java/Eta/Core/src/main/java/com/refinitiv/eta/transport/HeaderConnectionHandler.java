package com.refinitiv.eta.transport;

import java.util.List;

class HeaderConnectionHandler implements HeaderWebSocketHandler {

    private static final String CONNECTION_EXPECTED_VALUE = "Upgrade";

    @Override
    public int decodeWebSocketHeader(WebSocketSession session, HttpMessageHandler parser, HttpHeader header, Error error) {
        final List<String> headerValues = parser.getStringValues(header);
        if (headerValues.stream().anyMatch(value -> value.equals(CONNECTION_EXPECTED_VALUE))) {
            session.setConnectionUpgrade(true);
            return TransportReturnCodes.SUCCESS;
        }
        return populateError(error, TransportReturnCodes.FAILURE, "Header: %s, with value: %s hasn't contained value: %s",
                header.getHeaderName(), header.getSimpleHeaderValue(), CONNECTION_EXPECTED_VALUE);
    }

    @Override
    public int encodeWebSocketHeader(WebSocketSession webSocketSession, HttpHeaders httpHeaders, String headerName, Error error) {
        httpHeaders.addHeader(headerName, HttpHeaderLineInfo.valueOf(CONNECTION_EXPECTED_VALUE));
        return TransportReturnCodes.SUCCESS;
    }
}
