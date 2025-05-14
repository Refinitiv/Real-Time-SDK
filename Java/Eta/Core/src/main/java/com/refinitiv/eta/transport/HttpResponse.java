/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

class HttpResponse {

    private final HttpResponseConnectionInfo httpResponseConnectionInfo;

    private final HttpHeaders responseHeaders;

    private final Buffer responseBody;

    public HttpResponse() {
        this.httpResponseConnectionInfo = new HttpResponseConnectionInfo();
        this.responseHeaders = new HttpHeaders();
        this.responseBody = CodecFactory.createBuffer();
    }

    public HttpHeaders getResponseHeaders() {
        return responseHeaders;
    }

    public Buffer getResponseBody() {
        return responseBody;
    }

    public HttpResponseConnectionInfo getHttpResponseConnectionInfo() {
        return httpResponseConnectionInfo;
    }

    public void clear() {
        httpResponseConnectionInfo.clear();
        responseHeaders.clear();
        responseBody.clear();
    }

    public HttpMessage createHttpMessageResponse() {
        final HttpMessage cloned = new HttpMessage();
        httpResponseConnectionInfo.copy(cloned.getHttpResponseConnectionInfo());
        cloned.getCookies().putAll(responseHeaders.getCookies());
        this.responseBody.copy(cloned.getData());
        return cloned;
    }
}
