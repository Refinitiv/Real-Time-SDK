package com.refinitiv.eta.transport;

class HeaderUpgradeHandler implements HeaderWebSocketHandler {

    private static final String UPGRADE_EXPECTED_VALUE = "websocket";

    @Override
    public int decodeWebSocketHeader(WebSocketSession session, HttpMessageHandler parser, HttpHeader header, Error error) {
        if (UPGRADE_EXPECTED_VALUE.equalsIgnoreCase(header.getFirstHeaderValue())) {
            session.setUpgrade(true);
            return TransportReturnCodes.SUCCESS;
        }
        return populateError(error, TransportReturnCodes.FAILURE, "Header: %s, with value: %s hasn't contained value: %s",
                header.getHeaderName(), header.getSimpleHeaderValue(), UPGRADE_EXPECTED_VALUE);
    }

    @Override
    public int encodeWebSocketHeader(WebSocketSession session, HttpHeaders httpHeaders, String headerName, Error error) {
        httpHeaders.addHeader(headerName, HttpHeaderLineInfo.valueOf(UPGRADE_EXPECTED_VALUE));
        return TransportReturnCodes.SUCCESS;
    }
}
