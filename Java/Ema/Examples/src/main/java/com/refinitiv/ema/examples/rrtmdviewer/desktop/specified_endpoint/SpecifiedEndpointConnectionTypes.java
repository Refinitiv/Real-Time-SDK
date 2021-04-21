package com.refinitiv.ema.examples.rrtmdviewer.desktop.specified_endpoint;


public enum SpecifiedEndpointConnectionTypes {
    SOCKET("Socket"),
    WEBSOCKET("WebSocket"),
    ENCRYPTED_SOCKET("Encrypted-Socket"),
    ENCRYPTED_WEBSOCKET("Encrypted-WebSocket");

    private String textLabel;

    @Override
    public String toString() {
        return textLabel;
    }

    SpecifiedEndpointConnectionTypes(String textLabel) {
        this.textLabel = textLabel;
    }
}
