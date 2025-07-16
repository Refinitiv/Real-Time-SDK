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

/**
 * HttpMessage class.
 * @see HttpCallback
 */
public class HttpMessage {

    private HttpRequestConnectionInfo httpRequestConnectionInfo;

    private HttpResponseConnectionInfo httpResponseConnectionInfo;

    private List<HttpHeader> httpHeaders;

    private Map<String, String> cookies;

    private Buffer data;

    private Object userSpecObject;

    /**
     * Creates HttpMessage object.
     */
    public HttpMessage() {
        httpRequestConnectionInfo = new HttpRequestConnectionInfo();
        httpResponseConnectionInfo = new HttpResponseConnectionInfo();
        httpHeaders = new ArrayList<>();
        cookies = new HashMap<>();
        data = CodecFactory.createBuffer();
    }

    /**
     * Returns {@link HttpRequestConnectionInfo} information of HttpMessage.
     * 
     * @return HttpRequestConnectionInfo information of HttpMessage
     */
    public HttpRequestConnectionInfo getHttpRequestConnectionInfo() {
        return httpRequestConnectionInfo;
    }

    /**
     * Sets {@link HttpRequestConnectionInfo} information of HttpMessage.
     * 
     * @param httpRequestConnectionInfo the HttpRequestConnectionInfo information to set
     */
    public void setHttpRequestConnectionInfo(HttpRequestConnectionInfo httpRequestConnectionInfo) {
        this.httpRequestConnectionInfo = httpRequestConnectionInfo;
    }

    /**
     * Returns {@link HttpResponseConnectionInfo} information of HttpMessage.
     * 
     * @return HttpResponseConnectionInfo information of HttpMessage
     */
    public HttpResponseConnectionInfo getHttpResponseConnectionInfo() {
        return httpResponseConnectionInfo;
    }

    /**
     * Sets {@link HttpResponseConnectionInfo} information of HttpMessage.
     * 
     * @param httpResponseConnectionInfo HttpResponseConnectionInfo information of HttpMessage
     */
    public void setHttpResponseConnectionInfo(HttpResponseConnectionInfo httpResponseConnectionInfo) {
        this.httpResponseConnectionInfo = httpResponseConnectionInfo;
    }

    /**
     * Returns the list of {@link HttpHeader} information of HttpMessage.
     * 
     * @return list of HttpHeader
     */
    public List<HttpHeader> getHttpHeaders() {
        return httpHeaders;
    }

    /**
     * Sets the list of {@link HttpHeader} information of HttpMessage.
     * 
     * @param httpHeaders list of HttpHeader
     */
    public void setHttpHeaders(List<HttpHeader> httpHeaders) {
        this.httpHeaders = httpHeaders;
    }

    /**
     * Returns the string map of cookies information of HttpMessage.
     * Each entry is a name of the cookie and a value.
     * 
     * @return the string map name/value
     */
    public Map<String, String> getCookies() {
        return cookies;
    }

    /**
     * Sets the string map of cookies information of HttpMessage.
     * Each entry is a name of the cookie and a value.
     * 
     * @param cookies the string map name/value
     */
    public void setCookies(Map<String, String> cookies) {
        this.cookies = cookies;
    }

    /**
     * Returns the body data of HttpMessage.
     * 
     * @return the body data in a {link @Buffer}
     */
    public Buffer getData() {
        return data;
    }

    /**
     * Sets the body data of HttpMessage.
     * 
     * @param data the body data
     */
    public void setData(Buffer data) {
        this.data = data;
    }

    /**
     * Returns userSpecObject that was provided in {link @BindOptions} or {link @ConnectOptions}.
     * 
     * @return userSpecObject
     * @see BindOptions
     * @see ConnectOptions
     */
    public Object userSpecObject() {
        return this.userSpecObject;
    }

    /**
     * Sets userSpecObject.
     * 
     * @param userSpecObject the object
     */
    public void userSpecObject(Object userSpecObject) {
        this.userSpecObject = userSpecObject;
    }
}
