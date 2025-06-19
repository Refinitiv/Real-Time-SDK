/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.util.*;

class HttpHeaders {

    private final Map<String, HttpHeader> internalHeaderStorage = new HashMap<>();

    private final Map<String, String> cookies = new HashMap<>();

    private int headerGroupLength;

    public HttpHeader getHeader(Object key) {
        return getHeader(key, true);
    }

    public HttpHeader getHeader(Object key, boolean caseSensitive) {
        final String keyString = caseSensitive ? key.toString() : key.toString().toLowerCase();
        return internalHeaderStorage.get(keyString);
    }

    public boolean addHeader(String headerName, HttpHeaderLineInfo httpHeaderLineInfo) {
        return addHeader(headerName, httpHeaderLineInfo, true);
    }

    public boolean addHeader(String headerName, HttpHeaderLineInfo httpHeaderLineInfo, boolean caseSensitive) {
        String headerNameTmp = caseSensitive ? headerName : headerName.toLowerCase();
        final HttpHeader httpHeader = Optional
                .ofNullable(this.getHeader(headerNameTmp, caseSensitive))
                .orElseGet(() -> new HttpHeader(headerNameTmp));
        internalHeaderStorage.putIfAbsent(headerNameTmp, httpHeader);
        return httpHeader.getHeaderInfo().add(httpHeaderLineInfo);
    }

    public boolean containsHeader(Object key) {
        return internalHeaderStorage.containsKey(key.toString());
    }

    public Map<String, String> getCookies() {
        return cookies;
    }

    public void clear() {
        internalHeaderStorage.clear();
        cookies.clear();
        headerGroupLength = 0;
    }

    public Collection<HttpHeader> getHeaders() {
        return internalHeaderStorage.values();
    }

    public boolean isEmpty() {
        return internalHeaderStorage.isEmpty();
    }

    public int getHeaderGroupLength() {
        return headerGroupLength;
    }

    public void setHeaderGroupLength(int headerGroupLength) {
        this.headerGroupLength = headerGroupLength;
    }
}
