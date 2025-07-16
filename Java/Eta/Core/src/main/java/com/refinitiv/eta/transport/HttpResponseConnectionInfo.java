/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * HttpResponseConnectionInfo class.
 * 
 */
public class HttpResponseConnectionInfo {

    private String connectionLine;

    private int responseStatus;

    private String responseReasonText;

    private int contentLength;

    /**
     * Sets HTTP response connection line.
     * 
     * @param connectionLine connection line
     */
    public void setConnectionLine(String connectionLine) {
        this.connectionLine = connectionLine;
    }

    /**
     * Returns HTTP response connection line.
     * 
     * @return connection line
     */
    public String getConnectionLine() {
        return connectionLine;
    }

    /**
     * Returns HTTP response reason phrase.
     * 
     * @return response reason phrase
     */
    public String getResponseReasonText() {
        return responseReasonText;
    }

    /**
     * Sets HTTP response reason phrase.
     * 
     * @param responseReasonText response reason phrase
     */
    public void setResponseReasonText(String responseReasonText) {
        this.responseReasonText = responseReasonText;
    }

    /**
     * Returns HTTP response status code.
     * 
     * @return status code
     */
    public int getResponseStatus() {
        return responseStatus;
    }

    /**
     * Sets HTTP response status code.
     * 
     * @param responseStatus status code
     */
    public void setResponseStatus(int responseStatus) {
        this.responseStatus = responseStatus;
    }

    /**
     * Returns HTTP body length of the HTTP response.
     * 
     * @return HTTP body length
     */
    public int getContentLength() {
        return contentLength;
    }

    /**
     * Sets HTTP body length of the HTTP response.
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
        this.connectionLine = "";
        this.responseStatus = 0;
        this.contentLength = 0;
    }

    /**
     * Copies all information from another structure.
     * 
     * @param connectionInfo information to copy
     */
    public void copy(HttpResponseConnectionInfo connectionInfo) {
        connectionInfo.connectionLine = connectionLine;
        connectionInfo.responseStatus = responseStatus;
        connectionInfo.responseReasonText = responseReasonText;
        connectionInfo.contentLength = contentLength;
    }
}
