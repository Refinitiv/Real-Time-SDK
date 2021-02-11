package com.refinitiv.eta.transport;

interface HeaderWebSocketHandler {
    int decodeWebSocketHeader(WebSocketSession session, HttpMessageHandler parser, HttpHeader header, Error error);

    int encodeWebSocketHeader(WebSocketSession session, HttpHeaders httpHeaders, String headerName, Error error);

    default int populateError(Error error, int errorCode, String text, Object... formatStrings) {
        error.errorId(errorCode);
        error.text(String.format(text, formatStrings));
        return errorCode;
    }
}
