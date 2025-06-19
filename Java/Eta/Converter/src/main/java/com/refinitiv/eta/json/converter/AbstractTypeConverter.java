/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.RequestMsg;

abstract class AbstractTypeConverter {
    public static final boolean DEFAULT_BOOLEAN = false;
    public static final int DEFAULT_INT = -1;
    public static final String DEFAULT_TEXT = "";
    
    protected final JsonAbstractConverter converter;
    public int[] dataTypes;

    AbstractTypeConverter(JsonAbstractConverter converter) {
        this.converter = converter;
    }

    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        error.setError(JsonConverterErrorCodes.JSON_ERROR_OPERATION_NOT_SUPPORTED, this.getClass().getCanonicalName());
    }

    void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {
        error.setError(JsonConverterErrorCodes.JSON_ERROR_OPERATION_NOT_SUPPORTED, this.getClass().getCanonicalName());
    }

    boolean getBoolean(JsonNode node, String nodeName, JsonConverterError error) {
        if(node.isObject()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_TOKEN_TYPE, "token: " + nodeName + " was expected: PRIMITIVE, BOOLEAN, found: OBJECT");
            return DEFAULT_BOOLEAN;
        }

        if(node.isBoolean()) {
            return node.booleanValue();
        }
        if(node.isTextual() && node.asText().equals("true")) {
            return true;
        }
        if(node.isInt() && node.asInt() == 1) {
            return true;
        }

        return false;
    }

    String getText(JsonNode node, String nodeName, JsonConverterError error) {
        if(!node.isTextual()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_TOKEN_TYPE, "token: " + nodeName + " was expected TEXT, found:" + node.getNodeType());
            return DEFAULT_TEXT;
        }

        return node.asText();
    }

    int getInt(JsonNode node, String nodeName, JsonConverterError error) {
        if(!node.isInt()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_TOKEN_TYPE, "token: " + nodeName + " was expected INT, found:" + node.getNodeType());
            return DEFAULT_INT;
        }

        return node.intValue();
    }

    long getLong(JsonNode node, String nodeName, JsonConverterError error) {
        if (node.isInt()) {
            return node.intValue();
        } else if (node.isLong()) {
            return node.longValue();
        } else {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_TOKEN_TYPE, "token: " + nodeName + " was expected INT or LONG, found:" + node.getNodeType());
            return DEFAULT_INT;
        }
    }

    void checkInt(JsonNode node, String nodeName, JsonConverterError error) {
        if (!node.isInt())
            error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_TOKEN_TYPE, "token: " + nodeName + " Int was expected, found:" + node.getNodeType());
    }

    void checkStringOrInt(JsonNode node, String nodeName, JsonConverterError error) {
        if (!node.isInt() && !node.isTextual())
            error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_TOKEN_TYPE, "token: " + nodeName + " String or Int was expected, found:" + node.getNodeType());
    }

    void checkObject(JsonNode node, String nodeName, JsonConverterError error) {
        checkObject(node, nodeName, true, error);
    }

    void checkObject(JsonNode node, String nodeName, boolean objectIsExpected, JsonConverterError error) {
        if (objectIsExpected && !node.isObject()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_TOKEN_TYPE, "token: " + nodeName + " object was expected, found:" + node.getNodeType());
            return;
        }

        if (!objectIsExpected && node.isObject()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_TOKEN_TYPE, "token: " + nodeName + " object was not expected, found:" + node.getNodeType());
        }
    }

    void checkIpV4Address(String ipv4AddrString, JsonConverterError error) {
        int pos = 0;
        for (int i = 0; i < 4; i++) {
            pos = isCorrectIpV4Part(ipv4AddrString, pos);
            if (pos < 0) {
                break;
            }
        }
        if (pos < ipv4AddrString.length()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "invalid IpV4 value: " + ipv4AddrString, ConstCharArrays.JSON_ADDRESS);
            return;
        }
    }

    private int isCorrectIpV4Part(String ipv4AddrString, int start) {
        int res = 0;
        boolean foundDigit = false;
        while (start < ipv4AddrString.length() && ipv4AddrString.charAt(start) != '.') {
            int digit = ipv4AddrString.charAt(start) - '0';
            if ( digit >= 0 && digit <= 9) {
                foundDigit = true;
                res = res * 10 + digit;
                start++;
            } else
                return -1;
        }

        return foundDigit && res <= 255 ? start + 1 : -1;
    }

    void processUnexpectedKey(String key, JsonConverterError error) {
        if (converter.catchUnexpectedKeys())
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, key);
    }

    void setDeafultDynamicQoS(Object msg) {
        RequestMsg requestMsg = (RequestMsg) msg;
        if (!requestMsg.checkHasQos() && converter.useDefaultDynamicQoS()) {
            // No QOS was provided, set to dynamic if configured
            requestMsg.applyHasQos();
            requestMsg.qos().dynamic(true);
            requestMsg.qos().timeliness(QosTimeliness.REALTIME);
            requestMsg.qos().rate(QosRates.TICK_BY_TICK);

            requestMsg.applyHasWorstQos();
            requestMsg.worstQos().timeliness(QosTimeliness.DELAYED_UNKNOWN);
            requestMsg.worstQos().rate(QosRates.TIME_CONFLATED);
            requestMsg.worstQos().rateInfo(65535);
        }
    }

    int getEncodingMaxSize() {
        return 0;
    }
}
