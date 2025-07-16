/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.util.ArrayList;
import java.util.List;

/**
 * HttpHeader class.
 * @see HttpMessage
 */
public class HttpHeader {

    private String headerName;

    private StringBuilder headerValueBuilder = new StringBuilder();

    private String simpleHeaderValue;

    private final List<HttpHeaderLineInfo> headerInfo = new ArrayList<>();

    /**
     * Creates {@link HttpHeader} object with empty HTTP header name.
     */
    public HttpHeader(){
    }

    /**
     * Creates {@link HttpHeader} object with provided HTTP header name.
     * 
     * @param headerName HTTP header name
     */
    public HttpHeader(String headerName) {
        this.headerName = headerName;
    }

    /**
     * Returns HTTP header name of {@link HttpHeader} object.
     * 
     * @return HTTP header name
     */
    public String getHeaderName() {
        return headerName;
    }

    /**
     * Provides HTTP header name to {@link HttpHeader} object.
     * 
     * @param headerName HTTP header name
     */
    public void setHeaderName(String headerName) {
        this.headerName = headerName;
    }

    /**
     * Returns list of {@link HttpHeaderLineInfo} objects.
     * 
     * @return the list
     */
    public List<HttpHeaderLineInfo> getHeaderInfo() {
        return headerInfo;
    }

    /**
     * Provide string representation for the {@link HttpHeader} first value.
     * 
     * @return the String representation
     */
    public String getFirstHeaderValue() {
        if (!headerInfo.isEmpty()) {
            return headerInfo.get(0).getHeaderValue().trim();
        }
        return null;
    }

    /**
     * Provide string representation for {@link HttpHeader} values.
     * 
     * @return the String representation
     */
    public String getSimpleHeaderValue() {
        if (simpleHeaderValue == null) {
            generateSimpleHeaderValue();
        }
        return simpleHeaderValue;
    }

    void generateSimpleHeaderValue() {
        headerInfo.forEach(info -> headerValueBuilder.append(info.getHeaderValue()).append(info.getValueDelimiter()));
        headerValueBuilder.deleteCharAt(headerValueBuilder.length() - 1);
        simpleHeaderValue = headerValueBuilder.toString().trim();
    }

    /**
     * Provide string representation for {@link HttpHeader} name and values.
     * 
     * @return the String representation
     */
    @Override
    public String toString() {
        StringBuilder stringBuilder = new StringBuilder(headerName).append(": ");
        headerInfo.forEach(headerInfo -> stringBuilder
                .append(headerInfo.getHeaderValue())
                .append(headerInfo.getValueDelimiter())
        );
        return stringBuilder.toString();
    }

    /**
     * Copies all information from another Header object.
     * 
     * @param httpHeader object to copy
     */
    public void copy(HttpHeader httpHeader) {
        httpHeader.headerName = this.headerName;
        this.headerInfo.forEach(info -> httpHeader.getHeaderInfo().add(new HttpHeaderLineInfo(info)));
    }
}
