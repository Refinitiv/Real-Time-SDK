package com.refinitiv.eta.transport;

import java.util.List;

/**
 * Handler for header Sec-WebSocket-Extensions.
 */
abstract class HeaderSecWebSocketExtensionsHandler implements HeaderWebSocketHandler {

    protected static final String DEFLATE = "permessage-deflate";
    protected static final String SERVER_CONTEXT_EXTENSION = "server_no_context_takeover";
    protected static final String CLIENT_CONTEXT_EXTENSION = "client_no_context_takeover";
    protected static final char EXTENSIONS_VALUE_DELIMITER = ';';

    @Override
    public int decodeWebSocketHeader(WebSocketSession session, HttpMessageHandler parser, HttpHeader header, Error error) {
        if (session.hasCompressionSupport()) {
            final List<String> extensionValues = parser.getStringValues(header);
            for (String extension : extensionValues) {
                if (extension.equals(DEFLATE)) {
                    session.setDeflate(true);
                } else if (extension.equals(SERVER_CONTEXT_EXTENSION)) {
                	if(session.isClient)
                	{
                		session.applyNoInboundContextTakeOver();
                	}
                	else
                	{
                		session.applyNoOutboundContextTakeOver();
                	}
                } else if (extension.equals(CLIENT_CONTEXT_EXTENSION)) {
                	if(!session.isClient)
                	{
                		session.applyNoInboundContextTakeOver();
                	}
                	else
                	{
                		session.applyNoOutboundContextTakeOver();
                	}
                }
            }
        }
        return TransportReturnCodes.SUCCESS;
    }

    @Override
    public abstract int encodeWebSocketHeader(WebSocketSession session, HttpHeaders httpHeaders, String headerName, Error error);
}
