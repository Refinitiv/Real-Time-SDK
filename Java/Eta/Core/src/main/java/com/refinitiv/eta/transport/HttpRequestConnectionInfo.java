/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * HttpRequestConnectionInfo class.
 * 
 */
public class HttpRequestConnectionInfo {
    private String requestMethod;

    private String connectionLine;

    private String connectionUri;

    private int contentLength;

    /**
     * Returns HTTP request method name ("GET", "POST", ...).
     * 
     * @return request method
     */
    public String getRequestMethod() {
        return requestMethod;
    }

    /**
     * Sets HTTP request method name ("GET", "POST", ...).
     * 
     * @param requestMethod request method
     */
    public void setRequestMethod(String requestMethod) {
        this.requestMethod = requestMethod;
    }

    /**
     * Returns HTTP request connection line.
     * 
     * @return connection line
     */
    public String getConnectionLine() {
        return connectionLine;
    }

    /**
     * Sets HTTP request connection line.
     * 
     * @param connectionLine connection line
     */
    public void setConnectionLine(String connectionLine) {
        this.connectionLine = connectionLine;
    }

    /**
     * Returns HTTP request URI.
     * 
     * @return HTTP request URI
     */
    public String getConnectionUri() {
        return connectionUri;
    }

    /**
     * Sets HTTP request URI.
     * 
     * @param connectionUri HTTP request URI
     */
    public void setConnectionUri(String connectionUri) {
        this.connectionUri = connectionUri;
    }

    /**
     * Returns HTTP body length of the HTTP request.
     * 
     * @return HTTP body length
     */
    public int getContentLength() {
        return contentLength;
    }

    /**
     * Sets HTTP body length of the HTTP request.
     * 
     * @param contentLength the length
     */
    public void setContentLength(int contentLength) {
        this.contentLength = contentLength;
    }

    /**
     * Clears all information of the structure.
     */
    public void clear() {
        requestMethod = null;
        connectionLine = null;
        connectionUri = null;
        contentLength = 0;
    }

    /**
     * Copies all information from another structure.
     * 
     * @param connectionInfo information to copy
     */
    public void copy(HttpRequestConnectionInfo connectionInfo) {
        connectionInfo.requestMethod = this.requestMethod;
        connectionInfo.connectionLine = this.connectionLine;
        connectionInfo.connectionUri = this.connectionUri;
        connectionInfo.contentLength = this.contentLength;
    }
}
