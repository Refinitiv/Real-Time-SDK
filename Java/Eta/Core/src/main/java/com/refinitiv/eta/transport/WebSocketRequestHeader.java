/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.util.Arrays;
import java.util.List;
import java.util.Objects;

enum WebSocketRequestHeader {

    HOST("Host", true, new HostHeaderHandler()),

    UPGRADE("Upgrade", true, new HeaderUpgradeHandler()),

    CONNECTION("Connection", true, new HeaderConnectionHandler()),

    SEC_WEB_SOCKET_KEY("Sec-WebSocket-Key", true, new HeaderWebSocketSessionKeyHandler(false)),

    SEC_WEB_SOCKET_VERSION("Sec-WebSocket-Version", true, new SecWebSocketVersionHeaderHandler()),

    ORIGIN("Origin", false, new OriginHeaderHandler()),

    SEC_WEB_SOCKET_PROTOCOL("Sec-WebSocket-Protocol", true, new SecWebSocketProtocolHeaderHandler()),

    SEC_WEB_SOCKET_EXTENSIONS("Sec-WebSocket-Extensions", false, new RequestHeaderSecWebSocketExtensionsHandler());

    private final String headerTitle;

    private final String headerTitleInsensitiveCase;

    private final boolean required;

    private final HeaderWebSocketHandler webSocketHeaderHandler;

    WebSocketRequestHeader(String headerTitle, boolean required,
                           HeaderWebSocketHandler webSocketHeaderHandler) {
        this.headerTitle = headerTitle;
        this.headerTitleInsensitiveCase = headerTitle.toLowerCase();
        this.required = required;
        this.webSocketHeaderHandler = webSocketHeaderHandler;
    }

    public String getHeaderTitle() {
        return headerTitle;
    }

    public String getHeaderTitleInsensitiveCase() {
        return headerTitleInsensitiveCase;
    }

    public boolean isRequired() {
        return required;
    }

    public HeaderWebSocketHandler getWebSocketHeaderHandler() {
        return webSocketHeaderHandler;
    }

    /**
     * Handler for host header.
     */
    private static class HostHeaderHandler implements HeaderWebSocketHandler {

        private static final int MAX_HOST_HEADER_VALUES = 2;

        private static final String DEFAULT_HOST_VALUE = "localhost";

        @Override
        public int decodeWebSocketHeader(WebSocketSession webSocketSession, HttpMessageHandler httpParser, HttpHeader httpHeader, Error error) {
            final List<String> hostValues = httpParser.getStringValues(httpHeader);
            if (hostValues.isEmpty()) {
                populateError(error, TransportReturnCodes.FAILURE, "Error processing header: %s", httpHeader.toString());
                return TransportReturnCodes.FAILURE;
            }
            if (hostValues.size() > MAX_HOST_HEADER_VALUES) {
                populateError(error, TransportReturnCodes.FAILURE, "Too much values for host header: %s", hostValues.toString());
            }
            final String hostName = hostValues.get(0);
            webSocketSession.setHost(hostName);
            if (Objects.equals(hostValues.size(), MAX_HOST_HEADER_VALUES)) {
                final int hostPort = Integer.parseUnsignedInt(hostValues.get(1));
                webSocketSession.setPort(hostPort);
            }
            webSocketSession.setCompleteHostName(httpHeader.getFirstHeaderValue());
            return TransportReturnCodes.SUCCESS;
        }

        @Override
        public int encodeWebSocketHeader(WebSocketSession session, HttpHeaders httpHeaders, String headerName, Error error) {
            if (Objects.isNull(session.getHost())) {
                session.setHost(DEFAULT_HOST_VALUE);
            }
            String completeHostName = session.getHost();
            if (!Objects.equals( 0, session.getPort())) {
                completeHostName = completeHostName + ":" + session.getPort();
            }
            session.setCompleteHostName(completeHostName);
            httpHeaders.addHeader(headerName, HttpHeaderLineInfo.valueOf(completeHostName));
            return TransportReturnCodes.SUCCESS;
        }
    }

    /**
     * Handler for header Sec-WebSocket-Version.
     */
    private static class SecWebSocketVersionHeaderHandler implements HeaderWebSocketHandler {
        @Override
        public int decodeWebSocketHeader(WebSocketSession session, HttpMessageHandler parser, HttpHeader header, Error error) {
            try {
                session.setRecoveryVersion(Integer.parseUnsignedInt(header.getFirstHeaderValue()));
            } catch (Exception e) {
                return TransportReturnCodes.FAILURE;
            }
            return TransportReturnCodes.SUCCESS;
        }

        @Override
        public int encodeWebSocketHeader(WebSocketSession session, HttpHeaders httpHeaders, String headerName, Error error) {
            httpHeaders.addHeader(headerName, HttpHeaderLineInfo.valueOf(String.valueOf(session.getRecoveryVersion())));
            return TransportReturnCodes.SUCCESS;
        }
    }

    /**
     * Handler for header Sec-WebSocket-Protocols.
     */
    private static class SecWebSocketProtocolHeaderHandler implements HeaderWebSocketHandler {
        @Override
        public int decodeWebSocketHeader(WebSocketSession session, HttpMessageHandler parser, HttpHeader header, Error error) {
            final String protocols = session.getWebSocketOpts().protocols();
            final String acceptedProtocol = header.getHeaderInfo().stream()
                    .flatMap(httpHeaderLineInfo -> Arrays.stream(httpHeaderLineInfo.getHeaderValue().split(",")))
                    .map(String::trim)
                    .map(String::toLowerCase)
                    .filter(protocols::contains)
                    .findFirst()
                    .orElse(null);
            if (Objects.isNull(acceptedProtocol)) {
                return populateError(error, TransportReturnCodes.FAILURE,
                        "Application doesn't support these protocols: %s", protocols);
            }
            session.setProtocolVersion(acceptedProtocol);
            if (session.getAcceptedProtocol() == WebSocketSupportedProtocols.UNSUPPORTED_PROTOCOL) {
                return populateError(error, TransportReturnCodes.FAILURE,
                        "Cannot recognize the protocol type from provided protocol list: %s", protocols);
            }
            return TransportReturnCodes.SUCCESS;
        }

        @Override
        public int encodeWebSocketHeader(WebSocketSession session, HttpHeaders httpHeaders, String headerName, Error error) {
            final String protocols = session.getWebSocketOpts().protocols();
            if (protocols.isEmpty()) {
                return populateError(error, TransportReturnCodes.FAILURE, "Protocols wasn't passed.");
            }
            
            /* Sets to the protocol list in order to accept from the response later. */
            session.getWebSocketOpts().protocols(WebSocketHandlerImpl.constructProtocolList(protocols, false));
            
            httpHeaders.addHeader(headerName, HttpHeaderLineInfo.valueOf(session.getWebSocketOpts().protocols()));
            return TransportReturnCodes.SUCCESS;
        }
    }

    private static class OriginHeaderHandler implements HeaderWebSocketHandler {

        @Override
        public int decodeWebSocketHeader(WebSocketSession session, HttpMessageHandler parser, HttpHeader header, Error error) {
            session.setOrigin(header.getFirstHeaderValue());
            session.applyOriginSending();
            return TransportReturnCodes.SUCCESS;
        }

        @Override
        public int encodeWebSocketHeader(WebSocketSession session, HttpHeaders httpHeaders, String headerName, Error error) {
            if (session.hasOriginSending()) {
                httpHeaders.addHeader(headerName, HttpHeaderLineInfo.valueOf(session.getOrigin()));
            }
            return TransportReturnCodes.SUCCESS;
        }
    }

    private static class RequestHeaderSecWebSocketExtensionsHandler extends HeaderSecWebSocketExtensionsHandler {

        @Override
        public int encodeWebSocketHeader(WebSocketSession session, HttpHeaders httpHeaders, String headerName, Error error) {
        	
        	 if (!session.hasCompressionSupport()) {
                 return TransportReturnCodes.SUCCESS;
             }
        	
            httpHeaders.addHeader(headerName,
                    HttpHeaderLineInfo.builder().headerValue(DEFLATE)
                            .valueDelimiter(EXTENSIONS_VALUE_DELIMITER)
                            .build()
            );
            
            if(session.isClient)
            {
            	httpHeaders.addHeader(headerName,
            			HttpHeaderLineInfo.builder()
                            	.headerValue(SERVER_CONTEXT_EXTENSION)
                            	.valueDelimiter(EXTENSIONS_VALUE_DELIMITER)
                            	.build()
            			);
            }
            
            return TransportReturnCodes.SUCCESS;
        }
    }
}
