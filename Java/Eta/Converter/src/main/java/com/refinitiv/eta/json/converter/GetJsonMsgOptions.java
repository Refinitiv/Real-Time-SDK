/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

/**
 * Holds parameters passed to {@link JsonConverter#getJsonBuffer} method
 */
public interface GetJsonMsgOptions {

    /**
     * Resets the default values of the fields of the current {@link GetJsonMsgOptions} instance
     */
    void clear();

    /**
     * Setter for stream ID
     * @param streamId the value of the stream Id to be set
     * @return current {@link GetJsonMsgOptions} instance
     */
    GetJsonMsgOptionsImpl streamId(int streamId);

    /**
     * Setter for transport protocol type
     * @param protocol the value of the transport protocol type to be set
     * @return current {@link GetJsonMsgOptions} instance
     */
    GetJsonMsgOptionsImpl transportProtocolType(int protocol);

    /**
     * Setter for JSON protocol type
     * @param protocol the value of the JSON protocol type to be set
     * @return current {@link GetJsonMsgOptions} instance
     */
    GetJsonMsgOptionsImpl jsonProtocolType(int protocol);

    /**
     * Setter for the Solicited flag
     * @param solicited the value of the solicited flag to be set
     * @return current {@link GetJsonMsgOptions} instance
     */
    GetJsonMsgOptionsImpl isSolicited(boolean solicited);

    /**
     * Sets the value of the flag that determines whether a message is a Close message
     * @param isClose the value of the isClose flag
     * @return current {@link GetJsonMsgOptions} instance
     */
    GetJsonMsgOptionsImpl isCloseMsg(boolean isClose);

    /**
     * Getter for stream ID
     * @return current value of the stream Id
     */
    Integer getStreamId();

    /**
     * Getter for the transport protocol type
     * @return current value of the transport protocol type
     */
    int getTransportProtocol();

    /**
     * Getter for the json protocol type
     * @return current value of the json protocol type
     */
    int getJsonProtocolType();

    /**
     * Getter for the isClose
     * @return current value of isClose flag
     */
    boolean isCloseMsg();

    /**
     * Getter for the isSolicited
     * @return current value of isSolicited flag
     */
    boolean isSolicited();
}
