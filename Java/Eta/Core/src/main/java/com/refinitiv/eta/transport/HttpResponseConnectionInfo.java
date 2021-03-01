package com.refinitiv.eta.transport;

public class HttpResponseConnectionInfo {

    private String connectionLine;

    private int responseStatus;

    private String responseReasonText;

    private int contentLength;

    public void setConnectionLine(String connectionLine) {
        this.connectionLine = connectionLine;
    }

    public String getConnectionLine() {
        return connectionLine;
    }

    public String getResponseReasonText() {
        return responseReasonText;
    }

    public void setResponseReasonText(String responseReasonText) {
        this.responseReasonText = responseReasonText;
    }

    public int getResponseStatus() {
        return responseStatus;
    }

    public void setResponseStatus(int responseStatus) {
        this.responseStatus = responseStatus;
    }

    public int getContentLength() {
        return contentLength;
    }

    public void setContentLength(int contentLength) {
        this.contentLength = contentLength;
    }

    public void clear() {
        this.connectionLine = "";
        this.responseStatus = 0;
        this.contentLength = 0;
    }

    public void copy(HttpResponseConnectionInfo connectionInfo) {
        connectionInfo.connectionLine = connectionLine;
        connectionInfo.responseStatus = responseStatus;
        connectionInfo.responseReasonText = responseReasonText;
        connectionInfo.contentLength = contentLength;
    }
}