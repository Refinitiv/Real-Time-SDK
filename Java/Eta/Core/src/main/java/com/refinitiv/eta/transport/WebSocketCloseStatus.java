/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.util.Objects;

enum WebSocketCloseStatus {

    WS_CFSC_NORMAL_UNDEFINED(0, "Undefined"),

    /**
     * 1000 indicates a normal closure, meaning that the purpose for which the connection was established has been fulfilled.
     */
    WS_CFSC_NORMAL_CLOSE(1000, "Normal Closure"),

    /**
     * 1001 indicates that an endpoint is \"going away\", such as a server going down or a browser having navigated away from a page.
     */
    WS_CFSC_ENDPOINT_GONE(1001, "Going Away"),

    /**
     * 1002 indicates that an endpoint is terminating the connection due to a protocol error.
     */
    WS_CFSC_ENDPOINT_PROTOCOL_ERROR(1002, "Protocol Error"),

    /**
     * 1003 indicates that an endpoint is terminating the connection because it has received a type of data it cannot accept
     * (e.g., an endpoint that understands only text data MAY send this if it receives a binary message).
     */
    WS_CFSC_INVALID_FRAME_DATA_TYPE(1003, "Unsupported Data"),

    /**
     * 1004 "Reserved. The specific meaning might be defined in the future.
     */
    WS_CFSC_UNKNOWN_4(1004, "Reserved"),

    /**
     * 1005 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint. It is designated
     * for use in applications expecting a status code to indicate that no status code was actually present.
     */
    WS_CFSC_UNKNOWN_5(1005, "No Status Received"),

    /**
     * 1006 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint. It is designated
     * for use in applications expecting a status code to indicate that the connection was closed abnormally, e.g., without
     * sending or receiving a Close control frame.
     */
    WS_CFSC_UNKNOWN_6(1006, "Abnormal Closure"),

    /**
     * 1007 indicates that an endpoint is terminating the connection because it has received data within a message that was
     * not consistent with the type of the message (e.g., non-UTF-8 [RFC3629] data within a text message).
     */
    WS_CFSC_INVALID_CONT_FRAME_DATA(1007, "Invalid frame payload data"),

    /**
     * 1008 indicates that an endpoint is terminating the connection because it has received a message that violates its policy.
     * This is a generic status code that can be returned when there is no other more suitable status code (e.g., 1003 or 1009)
     * or if there is a need to hide specific details about the policy.
     */
    WS_CFSC_INVALID_FRAME(1008, "Policy Violation"),

    /**
     * 1009 indicates that an endpoint is terminating the connection because it has received a message that is too big for it
     * to process.
     */
    WS_CFSC_INVALID_FRAME_SIZE(1009, "Message Too Big"),

    /**
     * 1010 indicates that an endpoint (client) is terminating the connection because it has expected the server to negotiate
     * one or more extension, but the server didn't return them in the response message of the WebSocket handshake. The list
     * of extensions that are needed SHOULD appear in the /reason/ part of the Close frame.  Note that this status code is not
     * used by the server, because it can fail the WebSocket handshake instead.
     */
    WS_CFSC_INVALID_SERVER_EXTENSION(1010, "Mandatory Extension"),

    /**
     * 1011 indicates that a server is terminating the connection because it encountered an unexpected condition that prevented
     * it from fulfilling the request.
     */
    WS_CFSC_UNEXPECTED_EVENT(1011, "Internal Server Error"),

    /**
     * 1015 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint. It is designated
     * for use in applications expecting a status code to indicate that the connection was closed due to a failure to perform
     * a TLS handshake"
     */
    WS_CFSC_UNKNOWN_15(1015, "TLS Handshake");

    private int code;

    private String textMessage;

    private byte[] textMessageBytes;

    WebSocketCloseStatus(int code, String textMessage) {
        assert textMessage.length() <= (WebSocketFrameParser._WS_MAX_FRAME_LENGTH - WebSocketFrameParser._WS_CONTROL_HEADER_LEN);
        this.code = code;
        this.textMessage = textMessage;
        this.textMessageBytes = textMessage.getBytes();
    }

    public int getCode() {
        return code;
    }

    public String getTextMessage() {
        return textMessage;
    }

    public byte[] getTextMessageBytes() {
        return textMessageBytes;
    }

    public static WebSocketCloseStatus findByCode(int code) {
        if (Objects.equals(WS_CFSC_UNKNOWN_15.getCode(), code)) {
            return WS_CFSC_UNKNOWN_15;
        } else if (Objects.equals(WS_CFSC_NORMAL_UNDEFINED.getCode(), code)) {
            return WS_CFSC_NORMAL_UNDEFINED;
        }

        if (code > WS_CFSC_UNEXPECTED_EVENT.getCode() || code < WS_CFSC_NORMAL_CLOSE.getCode()) {
            return null;
        } else {
            return values()[(code % 1000) + 1];
        }
    }
}
