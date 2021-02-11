package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Codec;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

enum WebSocketSupportedProtocols {
    RWF("rssl.rwf"),
    JSON("rssl.json.v2", "tr_json2");

    public static final int UNSUPPORTED_PROTOCOL = -1;
    private static final String WEB_SOCKET_PROTOCOLS_SPLIT_REGEXP = "\\s*,\\s*";

    private final List<String> supportedProtocols;

    WebSocketSupportedProtocols(String... supportedProtocols) {
        this.supportedProtocols = Collections.unmodifiableList(Arrays.asList(supportedProtocols));
    }

    /**
     * Defines codec representation of string protocol.
     * @param protocol string representation of requested protocol.
     * @return integer representation of accepted protocol ({@link Codec#RWF_PROTOCOL_TYPE} or {@link Codec#JSON_PROTOCOL_TYPE}
     */
    public static int defineProtocol(String protocol) {
        if (RWF.supportedProtocols.contains(protocol)) {
            return Codec.RWF_PROTOCOL_TYPE;
        } else if (JSON.supportedProtocols.contains(protocol)) {
            return Codec.JSON_PROTOCOL_TYPE;
        }
        return UNSUPPORTED_PROTOCOL;
    }

    /**
     * Validates specified WebSocket sub protocols.
     * @param protocolList - list of WS sub protocols.
     * @return NULL if all specified protocols are valid or not specified or
     * returns invalid protocol which has been found during validation.
     */
    public static String validateProtocolList(String protocolList) {
        if (protocolList == null || protocolList.isEmpty()) {
            return null;
        }

        String[] protocols = protocolList.split(WEB_SOCKET_PROTOCOLS_SPLIT_REGEXP);
        for (String protocol : protocols) {
            if (WebSocketSupportedProtocols.defineProtocol(protocol) == WebSocketSupportedProtocols.UNSUPPORTED_PROTOCOL) {
                return protocol;
            }
        }
        return null;
    }
}
