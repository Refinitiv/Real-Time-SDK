package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

class HttpRequest {

    private final HttpRequestConnectionInfo httpRequestConnectionInfo;

    private final HttpHeaders requestHeaders;

    private final Buffer requestBody;

    public HttpRequest() {
        httpRequestConnectionInfo = new HttpRequestConnectionInfo();
        requestHeaders = new HttpHeaders();
        requestBody = CodecFactory.createBuffer();
    }

    public HttpRequestConnectionInfo getHttpRequestConnectionInfo() {
        return httpRequestConnectionInfo;
    }

    public HttpHeaders getRequestHeaders() {
        return requestHeaders;
    }

    public Buffer getRequestBody() {
        return requestBody;
    }

    public void clear() {
        httpRequestConnectionInfo.clear();
        requestHeaders.clear();
        requestBody.clear();
    }

    public HttpMessage createHttpMessageRequest() {
        final HttpMessage cloned = new HttpMessage();
        httpRequestConnectionInfo.copy(cloned.getHttpRequestConnectionInfo());
        cloned.getCookies().putAll(requestHeaders.getCookies());
        this.requestBody.copy(cloned.getData());
        return cloned;
    }
}