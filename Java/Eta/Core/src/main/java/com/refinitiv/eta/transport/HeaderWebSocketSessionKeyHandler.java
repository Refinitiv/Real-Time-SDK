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
import org.apache.commons.codec.Charsets;
import org.apache.commons.codec.digest.DigestUtils;

import java.nio.ByteBuffer;
import java.security.MessageDigest;
import java.util.Base64;
import java.util.List;
import java.util.Objects;
import java.util.UUID;

class HeaderWebSocketSessionKeyHandler implements HeaderWebSocketHandler {

    private static final String WEB_SOCKET_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    private static final MessageDigest SHA_MSG_DIGEST = DigestUtils.getSha1Digest();

    private static final Base64.Encoder BASE64_ENCODER = Base64.getEncoder();

    private final Buffer internalKeyBuffer;

    private final ByteBuffer uuidBuffer = ByteBuffer.allocate(16);

    private final boolean response;

    public HeaderWebSocketSessionKeyHandler(boolean response) {
        this.response = response;
        this.internalKeyBuffer = CodecFactory.createBuffer();
    }

    @Override
    public int decodeWebSocketHeader(WebSocketSession session, HttpMessageHandler parser, HttpHeader header, Error error) {
        if (header.getHeaderInfo().size() > 1) {
            return populateError(error, TransportReturnCodes.FAILURE, "Invalid HTTP header, duplicate fields received, Sec-Websocket-Key");
        }

        String wsKey = header.getFirstHeaderValue();
        if (Objects.isNull(wsKey) || wsKey.length() == 0) {
            return populateError(error, TransportReturnCodes.FAILURE, "WebSocket key cannot be empty.");
        }
        if (wsKey.startsWith("\"")) {
            final List<String> wsKeyHeaders = parser.getStringValues(header);
            if (wsKeyHeaders.size() != 1) {
                return populateError(error, TransportReturnCodes.FAILURE,
                        "Invalid format of web socket key header line: " + header.getSimpleHeaderValue());
            }
            wsKey = wsKeyHeaders.get(0);
        }

        final Buffer keyBuffer = response ? internalKeyBuffer : session.getWsSessionKey();
        keyBuffer.clear();
        keyBuffer.data(wsKey);
        if (response && !Objects.equals(session.getWsSessionAcceptKey(), internalKeyBuffer)) {
            return populateError(
                    error,
                    TransportReturnCodes.FAILURE,
                    "Key received: %s, is not key expected: %s",
                    session.getWsSessionAcceptKey().toString(),
                    session.getWsSessionKey().toString()
            );
        }
        return TransportReturnCodes.SUCCESS;
    }

    @Override
    public int encodeWebSocketHeader(WebSocketSession session, HttpHeaders httpHeaders, String headerName, Error error) {
        if (!response) {
            generateWebSocketSessionKey(session.getWsSessionKey());
        }
        byte[] concatenatedWebSocketAcceptKey = (session.getWsSessionKey() + WEB_SOCKET_GUID).getBytes(Charsets.US_ASCII);
        SHA_MSG_DIGEST.update(concatenatedWebSocketAcceptKey);
        final byte[] wsKeyAccepted = BASE64_ENCODER.encode(SHA_MSG_DIGEST.digest());
        session.getWsSessionAcceptKey().data(new String(wsKeyAccepted));
        final String wsKeyHeader = response ? session.getWsSessionAcceptKey().toString() : session.getWsSessionKey().toString();
        httpHeaders.addHeader(headerName, HttpHeaderLineInfo.valueOf(wsKeyHeader));
        return TransportReturnCodes.SUCCESS;
    }

    private void generateWebSocketSessionKey(Buffer webSocketSessionKeyBuffer) {
        final UUID uuid = UUID.randomUUID();
        uuidBuffer.clear();
        uuidBuffer
                .putLong(uuid.getMostSignificantBits())
                .putLong(uuid.getLeastSignificantBits());
        webSocketSessionKeyBuffer.data(BASE64_ENCODER.encodeToString(uuidBuffer.array()));
    }
}
