/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class HttpMessage {

    private HttpRequestConnectionInfo httpRequestConnectionInfo;

    private HttpResponseConnectionInfo httpResponseConnectionInfo;

    private List<HttpHeader> httpHeaders;

    private Map<String, String> cookies;

    private Buffer data;

    public HttpMessage() {
        httpRequestConnectionInfo = new HttpRequestConnectionInfo();
        httpResponseConnectionInfo = new HttpResponseConnectionInfo();
        httpHeaders = new ArrayList<>();
        cookies = new HashMap<>();
        data = CodecFactory.createBuffer();
    }

    public HttpRequestConnectionInfo getHttpRequestConnectionInfo() {
        return httpRequestConnectionInfo;
    }

    public void setHttpRequestConnectionInfo(HttpRequestConnectionInfo httpRequestConnectionInfo) {
        this.httpRequestConnectionInfo = httpRequestConnectionInfo;
    }

    public HttpResponseConnectionInfo getHttpResponseConnectionInfo() {
        return httpResponseConnectionInfo;
    }

    public void setHttpResponseConnectionInfo(HttpResponseConnectionInfo httpResponseConnectionInfo) {
        this.httpResponseConnectionInfo = httpResponseConnectionInfo;
    }

    public List<HttpHeader> getHttpHeaders() {
        return httpHeaders;
    }

    public void setHttpHeaders(List<HttpHeader> httpHeaders) {
        this.httpHeaders = httpHeaders;
    }

    public Map<String, String> getCookies() {
        return cookies;
    }

    public void setCookies(Map<String, String> cookies) {
        this.cookies = cookies;
    }

    public Buffer getData() {
        return data;
    }

    public void setData(Buffer data) {
        this.data = data;
    }
}
