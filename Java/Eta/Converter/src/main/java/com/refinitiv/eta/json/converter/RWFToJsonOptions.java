/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

/**
 * A {@link RWFToJsonOptions} instance carries parameters passed to {@link JsonConverter#convertRWFToJson} method
 */
public interface RWFToJsonOptions {

    /**
     * Getter for converter flags
     * @return RWFToJsonOptions flags
     */
    int getConverterFlags();

    /**
     * Setter for converter flags
     * @param converterFlags converter flags value to be set
     */
    void setConverterFlags(int converterFlags);

    /**
     * Getter for JSON protocol type
     * @return the type of the protocol of the current JSON message
     */
    int getJsonProtocolType();

    /**
     * Setter for json protocol type
     * @param jsonProtocolType the value of the json protocol type to be set
     */
    void setJsonProtocolType(int jsonProtocolType);

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
     * @return the current minor version of the wire
     */
    int getMinorVersion();

    /**
     * Setter for the minor version of the wire format to encode
     * @param minorVersion value of the minor version to be set
     */
    void setMinorVersion(int minorVersion);

    /**
     * Resets the default values of the fields of the current {@link RWFToJsonOptions} instance
     */
    void clear();
}
