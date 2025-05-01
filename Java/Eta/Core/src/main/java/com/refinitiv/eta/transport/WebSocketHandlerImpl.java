/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.net.URI;
import java.nio.ByteBuffer;
import java.util.Objects;

class WebSocketHandlerImpl implements WebSocketHandler {

    private enum WebSocketResponseStatusCode {
        SWITCHING_PROTOCOLS(101, "Switching Protocols"),
        BAD_REQUEST(400, "Bad Request"),
        UNAUTHORIZED(401, "Unauthorized"),
        PAYLOAD_TOO_LARGE(413, "Payload Too Large");

        private final int statusCode;
        private final String reasonText;

        private WebSocketResponseStatusCode(int statusCode, String reasonText) {
            this.statusCode = statusCode;
            this.reasonText = reasonText;
        }

        public int getStatusCode() {
            return statusCode;
        }

        public String getReasonText() {
            return reasonText;
        }
    }

    private static final String WS_REQUEST_HANDSHAKE_METHOD = "GET";

    private static final String SIMPLE_SCHEME = "ws";
    private static final String ENCRYPTED_SCHEME = "wss";
    private static final String WS_URI_SPECIFIC_PART = "websocket";
    
    static final String RSSL_JSON_V2 = "rssl.json.v2";
    static final String TR_JSON2 = "tr_json2";
    static final String RSSL_RWF = "rssl.rwf";

    private WebSocketSession wsSession;

    private HttpMessageHandler httpMessageHandler;

    private boolean initialized;

    public WebSocketHandlerImpl() {
        wsSession = new WebSocketSession();
    }

    @Override
    public void initialize() {
        wsSession.initialize();
        httpMessageHandler = new HttpMessageHandlerImpl();
        httpMessageHandler.initialize();
        initialized = true;
    }

    @Override
    public void clear() {
        wsSession.clear();
        if (httpMessageHandler != null)
            httpMessageHandler.clear();
    }

    @Override
    public boolean recognizeWebSocketHandshake(ByteBuffer data, int dataLength, int start, Error error) {
        int returnCode = httpMessageHandler.readRequestFirstLine(
                wsSession.getHandshakeOpeningRequest(), data, dataLength, start, error);
        if (returnCode < TransportReturnCodes.SUCCESS) { /* Fails to parse the received headers */
            return false;
        }
        return handleWebSocketUri(error) == TransportReturnCodes.SUCCESS;
    }

    @Override
    public int readOpeningHandshake(ByteBuffer data, int dataLength, int start, Error error) {
        getWsSession().getHandshakeOpeningRequest().clear();
        if (Objects.equals(dataLength, 0)) {
            return TransportReturnCodes.FAILURE;
        }
        int returnValue = httpMessageHandler.parseHttpRequest(wsSession.getHandshakeOpeningRequest(), data, dataLength, start, error);
        if (returnValue < TransportReturnCodes.SUCCESS) { /* Fails to parse the received headers */
            return returnValue;
        }

        returnValue = handleWebSocketUri(error);
        if (returnValue < TransportReturnCodes.SUCCESS) {
            return returnValue;
        }

        final HttpCallback callback = getWsSession().getWebSocketOpts().httpCallback();
        HttpMessage httpRequestMsg = null;
        if (Objects.nonNull(callback)) {
            httpRequestMsg = getWsSession().getHandshakeOpeningRequest().createHttpMessageRequest();
        }

        final HttpHeaders httpHeaders = wsSession.getHandshakeOpeningRequest().getRequestHeaders();
        //Get values regarding to another WebSocket headers.
        for (WebSocketRequestHeader requestHeader : WebSocketRequestHeader.values()) {
            /* Process the HTTP Header fields related to the Opening handshake received*/
            final HttpHeader httpHeader = httpHeaders.getHeader(requestHeader.getHeaderTitleInsensitiveCase());
            if (requestHeader.isRequired() && Objects.isNull(httpHeader)) {
                returnValue = populateError(error, TransportReturnCodes.FAILURE,
                        "Header title %s is not presented.", requestHeader.getHeaderTitle());
                break;
            }
            if (Objects.nonNull(httpHeader)) {
                returnValue = requestHeader.getWebSocketHeaderHandler()
                        .decodeWebSocketHeader(wsSession, httpMessageHandler, httpHeader, error);
                if (returnValue < TransportReturnCodes.SUCCESS) {
                    break;
                }
                //add current header to the callback msg.
                if (Objects.nonNull(httpRequestMsg)) {
                    final HttpHeader callbackHeader = new HttpHeader();
                    httpHeader.copy(callbackHeader);
                    httpRequestMsg.getHttpHeaders().add(callbackHeader);
                }
            }
        }
        if (Objects.nonNull(callback)) {
            getWsSession().getWebSocketOpts().httpCallback().httpCallback(httpRequestMsg, error);
        }
        return returnValue;
    }

    @Override
    public int createRequestHandshake(ByteBuffer data, Error error) {
        final HttpRequest httpRequest = wsSession.getHandshakeOpeningRequest();
        httpRequest.clear();
        final HttpRequestConnectionInfo connectionInfo = httpRequest.getHttpRequestConnectionInfo();

        int returnCode = validateUri(wsSession.getWebSocketUri(), error);
        if (returnCode < 0) {
            return TransportReturnCodes.FAILURE;
        }

        connectionInfo.setConnectionUri(wsSession.getWebSocketUri().toString());
        connectionInfo.setRequestMethod(WS_REQUEST_HANDSHAKE_METHOD);

        for (WebSocketRequestHeader requestHeader : WebSocketRequestHeader.values()) {
            returnCode = requestHeader.getWebSocketHeaderHandler()
                    .encodeWebSocketHeader(wsSession, httpRequest.getRequestHeaders(), requestHeader.getHeaderTitle(), error);
            if (requestHeader.isRequired() && returnCode < TransportReturnCodes.SUCCESS) {
                return returnCode;
            }
        }
        returnCode = httpMessageHandler.encodeRequestMessage(httpRequest, data, error);
        data.flip();
        return returnCode;
    }

    @Override
    public int readResponseHandshake(ByteBuffer data, int dataLength, int start, Error error) {
        if (Objects.equals(dataLength, 0)) {
            return TransportReturnCodes.FAILURE;
        }
        getWsSession().getHandshakeOpeningResponse().clear();
        int returnValue = httpMessageHandler.parseHttpResponse(wsSession.getHandshakeOpeningResponse(), data, dataLength, start, error);
        if (returnValue < TransportReturnCodes.SUCCESS) { /* Fails to parse the received headers */
            return returnValue;
        }

        final HttpCallback callback = getWsSession().getWebSocketOpts().httpCallback();
        HttpMessage httpResponseMessage = null;
        if (Objects.nonNull(callback)) {
            httpResponseMessage = getWsSession().getHandshakeOpeningResponse().createHttpMessageResponse();
        }
        final HttpHeaders httpHeaders = wsSession.getHandshakeOpeningResponse().getResponseHeaders();
        for (WebSocketResponseHeader responseHeader : WebSocketResponseHeader.values()) {
            final HttpHeader httpHeader = httpHeaders.getHeader(responseHeader.getHeaderTitleCaseInsensitive());
            if (responseHeader.isRequired() && Objects.isNull(httpHeader)) {
                returnValue = populateError(error, TransportReturnCodes.FAILURE,
                        "Header title %s is not presented.", responseHeader.getHeaderTitle());
                break;
            }
            if (Objects.nonNull(httpHeader)) {
                returnValue = responseHeader.getWebSocketHeaderHandler()
                        .decodeWebSocketHeader(wsSession, httpMessageHandler, httpHeader, error);
                if (returnValue < TransportReturnCodes.SUCCESS) {
                    break;
                }

                //add header to the response msg callback
                if (Objects.nonNull(httpResponseMessage)) {
                    final HttpHeader callbackHeader = new HttpHeader();
                    httpHeader.copy(callbackHeader);
                    httpResponseMessage.getHttpHeaders().add(callbackHeader);
                }
            }
        }
        if (Objects.nonNull(callback)) {
            getWsSession().getWebSocketOpts().httpCallback().httpCallback(httpResponseMessage, error);
        }
        return returnValue;
    }

    @Override
    public int createResponseHandshake(ByteBuffer data, Error error) {
        final HttpResponse httpResponse = wsSession.getHandshakeOpeningResponse();
        httpResponse.clear();
        final HttpResponseConnectionInfo connectionInfo = httpResponse.getHttpResponseConnectionInfo();
        connectionInfo.setResponseStatus(WebSocketResponseStatusCode.SWITCHING_PROTOCOLS.getStatusCode());
        connectionInfo.setResponseReasonText(WebSocketResponseStatusCode.SWITCHING_PROTOCOLS.getReasonText());
        int returnCode;
        for (WebSocketResponseHeader responseHeader : WebSocketResponseHeader.values()) {
            returnCode = responseHeader.getWebSocketHeaderHandler()
                    .encodeWebSocketHeader(wsSession, httpResponse.getResponseHeaders(), responseHeader.getHeaderTitle(), error);
            if (responseHeader.isRequired() && returnCode < TransportReturnCodes.SUCCESS) {
                return returnCode;
            }
        }
        returnCode = httpMessageHandler.encodeResponseMessage(httpResponse, data, error);
        data.flip();
        return returnCode;
    }

    @Override
    public int createBadRequestErrorResponse(ByteBuffer data, Error error) {
        return createErrorResponse(data, error, WebSocketResponseStatusCode.BAD_REQUEST);
    }

    @Override
    public int createUnauthorizedErrorResponse(ByteBuffer data, Error error) {
        return createErrorResponse(data, error, WebSocketResponseStatusCode.UNAUTHORIZED);
    }

    @Override
    public int createPayloadLargeErrorResponse(ByteBuffer data, Error error) {
        return createErrorResponse(data, error, WebSocketResponseStatusCode.PAYLOAD_TOO_LARGE);
    }

    private int createErrorResponse(ByteBuffer data, Error error, WebSocketResponseStatusCode code) {
        final HttpResponse httpResponse = wsSession.getHandshakeOpeningResponse();
        httpResponse.clear();
        final HttpResponseConnectionInfo connectionInfo = httpResponse.getHttpResponseConnectionInfo();
        connectionInfo.setResponseStatus(code.getStatusCode());
        connectionInfo.setResponseReasonText(code.getReasonText());
        final HttpHeaderLineInfo.HttpHeaderLineInfoBuilder builder = HttpHeaderLineInfo.builder();
        httpResponse.getResponseHeaders().addHeader("Content-Type", builder.headerValue("text/html; charset=UTF-8").build());
        httpResponse.getResponseHeaders().addHeader("Cache-Control", builder.headerValue("no-cache, private, no-store").build());
        httpResponse.getResponseHeaders().addHeader("Transfer-Encoding", builder.headerValue("chunked").build());
        httpResponse.getResponseHeaders().addHeader("Connection", builder.headerValue("close").build());
        int returnCode = httpMessageHandler.encodeResponseMessage(httpResponse, data, error);
        data.flip();
        return returnCode;
    }

    private int handleWebSocketUri(Error error) {
        // Check the first line is a GET request and the field-value field
        // has a URL for /WebSocket
        // e.g. value = '/WebSocket HTTP/1.1' fv_WebSocketURI = '/WebSocket'
        final HttpRequestConnectionInfo connectionInfo = wsSession.getHandshakeOpeningRequest().getHttpRequestConnectionInfo();
        if (!WS_REQUEST_HANDSHAKE_METHOD.equalsIgnoreCase(connectionInfo.getRequestMethod())) {
        	return TransportReturnCodes.FAILURE;
        }
        final URI webSocketUri = URI.create(connectionInfo.getConnectionUri());
        if (validateUri(webSocketUri, error) < 0) {
            return TransportReturnCodes.FAILURE;
        }
        wsSession.setWebSocketUri(webSocketUri);
        return TransportReturnCodes.SUCCESS;
    }

    private int validateUri(URI webSocketUri, Error error) {
        final String webSocketUriScheme = webSocketUri.getScheme();
        if (Objects.nonNull(webSocketUriScheme)) {
            if (Objects.equals(ENCRYPTED_SCHEME, webSocketUriScheme)) {
                wsSession.setEncrypted(true);
            } else if (!Objects.equals(SIMPLE_SCHEME, webSocketUriScheme)) {
                return populateError(error, TransportReturnCodes.FAILURE,
                        "Specified URI scheme is not supported for WebSocket handshake: " + webSocketUriScheme);
            }
        }
        final String uriSchemeSpecificPart = webSocketUri.getSchemeSpecificPart();
        if (!uriSchemeSpecificPart.toLowerCase().contains(WS_URI_SPECIFIC_PART)) {
            return populateError(error, TransportReturnCodes.FAILURE,
                    "Invalid URI for WebSocketHandshake: " + uriSchemeSpecificPart);
        }
        return TransportReturnCodes.SUCCESS;
    }

    private int populateError(Error error, int errorCode, String text, Object... formatStrings) {
        error.errorId(errorCode);
        error.text(String.format(text, formatStrings));
        return errorCode;
    }

    public WebSocketSession getWsSession() {
        return wsSession;
    }

    @Override
    public boolean initialized() {
        return initialized;
    }

    @Override
    public void loadWebSocketOpts(WSocketOpts wSocketOpts) {
        setWsProtocols(wSocketOpts, false);
    }

    @Override
    public void loadServerWebSocketOpts(WSocketOpts wSocketOpts) {
        setWsProtocols(wSocketOpts, true);
    }

    private void setWsProtocols(WSocketOpts wSocketOpts, boolean server) {
        final String protocols = wSocketOpts.protocols();
        if (protocols == null || protocols.isEmpty()) {
            wSocketOpts.protocols("");
        }
        wsSession.setWebSocketOpts(wSocketOpts);
    }
    
    public static String constructProtocolList(String protocols, boolean isServer)
    {
    	final String[] protocolList = protocols.split("[\\s,]+");
    	StringBuilder protocolBuilder = new StringBuilder();
    	boolean addedJSON2 = false, addedRSSL_RWF = false;
    	for(String protName : protocolList)
    	{
    		protName = protName.trim();

    		if(!addedJSON2 && !addedRSSL_RWF)
    		{
    			if (protName.equalsIgnoreCase(TR_JSON2))
    			{
    				addedJSON2 = true;
    				protocolBuilder.append(protName);

    				/* Added support for both tr_json2 and rssl.json.v2 on the server side to accept either one of them. */
    				if(isServer)
    				{
    					protocolBuilder.append(", " + RSSL_JSON_V2);
    				}
    			}
    			else if(protName.equalsIgnoreCase(RSSL_JSON_V2))
    			{
    				protName = TR_JSON2;
    				addedJSON2 = true;
    				protocolBuilder.append(protName);

    				if(isServer)
    				{
    					protocolBuilder.append(", " + RSSL_JSON_V2);
    				}
    			}
    			else if(protName.equalsIgnoreCase(RSSL_RWF))
    			{
    				addedRSSL_RWF = true;
    				protocolBuilder.append(protName);
    			}
    		}
    		else
    		{
    			if(!addedJSON2)
    			{
    				if (protName.equalsIgnoreCase(TR_JSON2))
    				{
    					addedJSON2 = true;
    					protocolBuilder.append(", " + protName);

    					if(isServer)
    					{
    						protocolBuilder.append(", " + RSSL_JSON_V2);
    					}
    				}
    				else if(protName.equalsIgnoreCase(RSSL_JSON_V2))
    				{
    					protName = TR_JSON2;
    					addedJSON2 = true;

    					protocolBuilder.append(", " + protName);

    					if(isServer)
    					{
    						protocolBuilder.append(", " + RSSL_JSON_V2);
    					}
    				}
    			}
    			else if(!addedRSSL_RWF && protName.equalsIgnoreCase(RSSL_RWF))
    			{
    				addedRSSL_RWF = true;
    				protocolBuilder.append(", " + protName);
    			}
    		}
    	}

    	return protocolBuilder.toString();
    }
}
