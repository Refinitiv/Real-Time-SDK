package com.refinitiv.eta.transport;

@FunctionalInterface
interface HeaderWebSocketEncoder {
    int encodeWebSocketHeader(WebSocketSession webSocketSession);
}

