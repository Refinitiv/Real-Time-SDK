/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.util.ArrayList;
import java.util.List;

public class HttpHeader {

    private String headerName;

    private StringBuilder headerValueBuilder = new StringBuilder();

    private String simpleHeaderValue;

    private final List<HttpHeaderLineInfo> headerInfo = new ArrayList<>();

    public HttpHeader(){
    }

    public HttpHeader(String headerName) {
        this.headerName = headerName;
    }

    public String getHeaderName() {
        return headerName;
    }

    public void setHeaderName(String headerName) {
        this.headerName = headerName;
    }

    public List<HttpHeaderLineInfo> getHeaderInfo() {
        return headerInfo;
    }

    public String getFirstHeaderValue() {
        if (!headerInfo.isEmpty()) {
            return headerInfo.get(0).getHeaderValue().trim();
        }
        return null;
    }

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

    @Override
    public String toString() {
        StringBuilder stringBuilder = new StringBuilder(headerName).append(": ");
        headerInfo.forEach(headerInfo -> stringBuilder
                .append(headerInfo.getHeaderValue())
                .append(headerInfo.getValueDelimiter())
        );
        return stringBuilder.toString();
    }

    public void copy(HttpHeader httpHeader) {
        httpHeader.headerName = this.headerName;
        this.headerInfo.forEach(info -> httpHeader.getHeaderInfo().add(new HttpHeaderLineInfo(info)));
    }
}
