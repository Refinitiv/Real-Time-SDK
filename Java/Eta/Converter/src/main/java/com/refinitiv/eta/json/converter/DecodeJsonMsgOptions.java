/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

/**
 * Holds the parameters passed on to {@link JsonConverter#decodeJsonMsg} method
 */
public interface DecodeJsonMsgOptions {

    /**
     * Getter for rssl protocol type
     * @return the type of the rssl protocol of the current JSON message
     */
    int getRsslProtocolType();

    /**
     * Setter for rssl protocol type
     * @param rsslProtocolType the value of the rssl protocol type to be set
     */
    void setRsslProtocolType(int rsslProtocolType);

    /**
     * Getter for the major version of the wire format to encode
     * @return current major version of the wire format
     */
    int getMajorVersion();

    /**
     * Setter for the major version of the wire format to encode
     * @param majorVersion value of the major version to be set
     */
    void setMajorVersion(int majorVersion);

    /**
     * Getter for the minor version of the wire format to encode
     * @return the current minor version of the wire format
     */
    int getMinorVersion();

    /**
     * Setter for the minor version of the wire format to encode
     * @param minorVersion value of the minor version to be set
     */
    void setMinorVersion(int minorVersion);

    /**
     * Getter for protocol type
     * @return the type of the protocol of the current JSON message
     */
    int getJsonProtocolType();

    /**
     * Setter for json protocol type
     * @param jsonProtocolType the value of the json protocol type to be set
     */
    void setJsonProtocolType(int jsonProtocolType);

    /**
     * Getter for converter flags
     * @return DecodeJsonMsgOptions flags
     */
    int getConverterFlags();

    /**
     * Setter for converter flags
     * @param converterFlags converter flags value to be set
     */
    void setConverterFlags(int converterFlags);

    /**
     * Resets the default values of the fields of the current {@link DecodeJsonMsgOptions} instance
     */
    void clear();
}
