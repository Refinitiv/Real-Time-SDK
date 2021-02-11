package com.refinitiv.eta.transport;

public class HttpRequestConnectionInfo {
    private String requestMethod;

    private String connectionLine;

    private String connectionUri;

    private int contentLength;

    public String getRequestMethod() {
        return requestMethod;
    }

    public void setRequestMethod(String requestMethod) {
        this.requestMethod = requestMethod;
    }

    public String getConnectionLine() {
        return connectionLine;
    }

    public void setConnectionLine(String connectionLine) {
        this.connectionLine = connectionLine;
    }

    public String getConnectionUri() {
        return connectionUri;
    }

    public void setConnectionUri(String connectionUri) {
        this.connectionUri = connectionUri;
    }

    public int getContentLength() {
        return contentLength;
    }

    public void setContentLength(int contentLength) {
        this.contentLength = contentLength;
    }

    public void clear() {
        requestMethod = null;
        connectionLine = null;
        connectionUri = null;
        contentLength = 0;
    }

    public void copy(HttpRequestConnectionInfo connectionInfo) {
        connectionInfo.requestMethod = this.requestMethod;
        connectionInfo.connectionLine = this.connectionLine;
        connectionInfo.connectionUri = this.connectionUri;
        connectionInfo.contentLength = this.contentLength;
    }
}
