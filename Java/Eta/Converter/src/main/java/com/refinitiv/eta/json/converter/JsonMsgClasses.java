/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

public class JsonMsgClasses {

    public static final String RSSL_MESSAGE_STR = "RSSL Message";
    public static final String PING_STR = "Ping";
    public static final String PONG_STR = "Pong";
    public static final String ERROR_STR = "Error";


    /**
     * Rwf Converted msg.
     */
    public static final int RSSL_MESSAGE = 1;

    /**
     * JSON Ping msg.
     */
    public static final int PING = 2;

    /**
     * JSON Pong msg.
     */
    public static final int PONG = 3;

    /**
     * JSON Error msg.
     */
    public static final int ERROR = 4;

    public static String toString(int jsonMsgClass) {
        switch (jsonMsgClass) {
            case RSSL_MESSAGE:
                return RSSL_MESSAGE_STR;
            case PING:
                return PING_STR;
            case PONG:
                return PONG_STR;
            case ERROR:
                return ERROR_STR;
        }
        return "";
    }
}
