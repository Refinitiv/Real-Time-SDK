/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public enum JsonMsgTypes {
    PING("Ping", 1),
    PONG("Pong", 2),
    ERROR("Error", 3);

    private static final Map<String, JsonMsgTypes> MSG_TYPES_MAP = new HashMap<>();

    static {
        Arrays.stream(values()).forEach(value -> MSG_TYPES_MAP.put(value.typeName, value));
    }

    private String typeName;
    private int readCode;

    JsonMsgTypes(String typeName, int readCode) {
        this.typeName = typeName;
        this.readCode = readCode;
    }

    public String getTypeName() {
        return typeName;
    }

    public int getReadCode() {
        return readCode;
    }

    public static JsonMsgTypes getJsonMsgByType(String type) {
        return MSG_TYPES_MAP.get(type);
    }

    public static JsonMsgTypes getJsonMsgByCode(int readCode) {
        if (readCode > JsonMsgTypes.ERROR.getReadCode() || readCode < JsonMsgTypes.PING.getReadCode()) {
            return null;
        }
        return values()[readCode];
    }
}
