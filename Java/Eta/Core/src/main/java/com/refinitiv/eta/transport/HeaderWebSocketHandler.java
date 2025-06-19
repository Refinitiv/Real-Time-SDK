/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

interface HeaderWebSocketHandler {
    int decodeWebSocketHeader(WebSocketSession session, HttpMessageHandler parser, HttpHeader header, Error error);

    int encodeWebSocketHeader(WebSocketSession session, HttpHeaders httpHeaders, String headerName, Error error);

    default int populateError(Error error, int errorCode, String text, Object... formatStrings) {
        error.errorId(errorCode);
        error.text(String.format(text, formatStrings));
        return errorCode;
    }
}
