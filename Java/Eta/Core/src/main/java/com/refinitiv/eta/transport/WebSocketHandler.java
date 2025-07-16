/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.nio.ByteBuffer;

interface WebSocketHandler {

    /**
     * Initialize required data of web socket handler (including the web socket session).
     */
    void initialize();

    /**
     * Clear all data of web socket handler (including the web socket session)
     */
    void clear();

    /**
     * Try to read the first line of request.
     * @param data       - whole received data
     * @param dataLength - length of request in buffer.
     * @param start      - start position of request in buffer.
     * @param error      - error.
     * @return true if it is websocket opening handshake.
     * Will return false if it is wrong.
     */
    boolean recognizeWebSocketHandshake(ByteBuffer data, int dataLength, int start, Error error);

    /**
     * @param data       - whole received data
     * @param dataLength - length of request in buffer.
     * @param start      - start position of request in buffer.
     * @param error      - error.
     * @return {@link TransportReturnCodes#SUCCESS} if websocket opening handshake is contained properly data.
     * Will return {@link TransportReturnCodes#FAILURE} if particular data is wrong.
     */
    int readOpeningHandshake(ByteBuffer data, int dataLength, int start, Error error);

    /**
     * @param data - data into which byte content of response should be written.
     * @param error - error.
     * @return {@link TransportReturnCodes#SUCCESS} if creation of request handshake is finished without errors.
     * Will return {@link TransportReturnCodes#FAILURE} if creation of request handshake isn't completely ended.
     */
    int createRequestHandshake(ByteBuffer data, Error error);

    /**
     * @param data       - whole received data.
     * @param dataLength - length of response in buffer.
     * @param start      - start position of response in buffer.
     * @param error      - error.
     * @return {@link TransportReturnCodes#SUCCESS} if websocket opening handshake is contained properly data.
     * Will return {@link TransportReturnCodes#FAILURE} if particular data is wrong.
     */
    int readResponseHandshake(ByteBuffer data, int dataLength, int start, Error error);

    /**
     * @param data  - data into which byte content of response should be written.
     * @param error - error.
     * @return {@link TransportReturnCodes#SUCCESS} if creation of response handshake is finished without errors.
     * Will return {@link TransportReturnCodes#FAILURE} if creation of response handshake isn't completely ended.
     */
    int createResponseHandshake(ByteBuffer data, Error error);

    int createBadRequestErrorResponse(ByteBuffer data, Error error);

    int createUnauthorizedErrorResponse(ByteBuffer data, Error error);

    int createPayloadLargeErrorResponse(ByteBuffer data, Error error);

    /**
     * @return returns actual instance of WebSocket session.
     */
    WebSocketSession getWsSession();

    boolean initialized();

    /**
     * Loads specified {@link WSocketOpts} to the current {@link WebSocketSession}
     *
     * @param wSocketOpts - opts which should be added to the current {@link WebSocketSession} for client.
     */
    void loadWebSocketOpts(WSocketOpts wSocketOpts);

    /**
     * Loads specified {@link WSocketOpts} to the current {@link WebSocketSession} for server.
     *
     * @param wSocketOpts - opts which should be added to the current {@link WebSocketSession}
     */
    void loadServerWebSocketOpts(WSocketOpts wSocketOpts);

    /**
     * @param userSpecObject - the userSpecObject to set.
     * @see BindOptions
     * @see ConnectOptions
     */
    void setUserSpecObject(Object userSpecObject);
}
