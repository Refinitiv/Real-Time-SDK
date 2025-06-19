/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.util.Objects;

enum WebSocketResponseHeader {
    CONNECTION("Connection", true, new HeaderConnectionHandler()),
    UPGRADE("Upgrade", true, new HeaderUpgradeHandler()),
    SEC_WEB_SOCKET_ACCEPT("Sec-WebSocket-Accept", true, new HeaderWebSocketSessionKeyHandler(true)),
    SEC_WEB_SOCKET_PROTOCOL("Sec-WebSocket-Protocol", true, new SecWebSocketProtocolHeaderHandler()),
    SEC_WEB_SOCKET_EXTENSIONS("Sec-WebSocket-Extensions", false, new ResponseHeaderSecWebSocketExtensionsHandler());

    private final String headerTitle;

    private final String headerTitleCaseInsensitive;

    private final boolean required;

    private final HeaderWebSocketHandler webSocketHeaderHandler;

    WebSocketResponseHeader(String headerTitle, boolean required, HeaderWebSocketHandler webSocketHeaderHandler) {
        this.headerTitle = headerTitle;
        this.headerTitleCaseInsensitive = headerTitle.toLowerCase();
        this.required = required;
        this.webSocketHeaderHandler = webSocketHeaderHandler;
    }

    public String getHeaderTitle() {
        return headerTitle;
    }

    public String getHeaderTitleCaseInsensitive() {
        return headerTitleCaseInsensitive;
    }

    public boolean isRequired() {
        return required;
    }

    public HeaderWebSocketHandler getWebSocketHeaderHandler() {
        return webSocketHeaderHandler;
    }

    private static class SecWebSocketProtocolHeaderHandler implements HeaderWebSocketHandler {

        private HttpHeaderLineInfo.HttpHeaderLineInfoBuilder lineBuilder = HttpHeaderLineInfo.builder();

        @Override
        public int decodeWebSocketHeader(WebSocketSession session, HttpMessageHandler parser, HttpHeader header, Error error) {
            /*If client hasn't send protocol, the protocol had been set automatically, so we should ignore validation*/
            if (Objects.nonNull(session.getProtocolName())) {
                return TransportReturnCodes.SUCCESS;
            }

            /*Validate received protocol.*/
            final String acceptedProtocol = header.getSimpleHeaderValue();
            if (acceptedProtocol.contains(",")) {
                return populateError(error, TransportReturnCodes.FAILURE, "Cannot accept multiple protocols: %s", acceptedProtocol);
            }
            final String protocols = session.getWebSocketOpts().protocols();
            if (!protocols.contains(acceptedProtocol)) {
                return populateError(error, TransportReturnCodes.FAILURE, "Accepted protocol: %s, is not supported by app", acceptedProtocol);
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
            if (Objects.isNull(session.getProtocolName())) {
                return TransportReturnCodes.FAILURE;
            }
            httpHeaders.addHeader(headerName, lineBuilder.headerValue(session.getProtocolName()).build());
            return TransportReturnCodes.SUCCESS;
        }
    }

    private static class ResponseHeaderSecWebSocketExtensionsHandler extends HeaderSecWebSocketExtensionsHandler {
        @Override
        public int encodeWebSocketHeader(WebSocketSession session, HttpHeaders httpHeaders, String headerName, Error error) {
            if (!session.hasCompressionSupport()) {
                return TransportReturnCodes.SUCCESS;
            }

            if (session.isDeflate()) {
                httpHeaders.addHeader(headerName,
                        HttpHeaderLineInfo.builder().headerValue(DEFLATE)
                                .valueDelimiter(EXTENSIONS_VALUE_DELIMITER)
                                .build()
                );
                if (session.hasNoOutboundContext()) {
                    httpHeaders.addHeader(headerName,
                            HttpHeaderLineInfo.builder()
                                    .headerValue(SERVER_CONTEXT_EXTENSION)
                                    .valueDelimiter(EXTENSIONS_VALUE_DELIMITER)
                                    .build()
                    );
                }

                if (session.hasNoInboundContext()) {
                    httpHeaders.addHeader(headerName,
                            HttpHeaderLineInfo.builder()
                                    .headerValue(CLIENT_CONTEXT_EXTENSION)
                                    .valueDelimiter(EXTENSIONS_VALUE_DELIMITER)
                                    .build()
                    );
                }
            }
            return TransportReturnCodes.SUCCESS;
        }
    }
}
