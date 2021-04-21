package com.refinitiv.ema.examples.rrtmdviewer.desktop.common;

public enum SupportedConnectionTypes {
    SOCKET("Socket"),
    WEBSOCKET("WebSocket"),
    ENCRYPTED_SOCKET("Encrypted-Socket"),
    ENCRYPTED_WEBSOCKET("Encrypted-WebSocket");

    private String textLabel;

    SupportedConnectionTypes(String textLabel) {
        this.textLabel = textLabel;
    }

    @Override
    public String toString() {
        return textLabel;
    }
}
