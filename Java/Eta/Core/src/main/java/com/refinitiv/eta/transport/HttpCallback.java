package com.refinitiv.eta.transport;

public interface HttpCallback {
    void httpCallback(HttpMessage httpMessage, Error error);
}
