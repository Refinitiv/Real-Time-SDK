/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

class RWFToJsonOptionsImpl implements RWFToJsonOptions {

    private int converterFlags;
    private int JsonProtocolType;
    private int majorVersion;
    private int minorVersion;

    RWFToJsonOptionsImpl() {}

    public int getConverterFlags() {
        return converterFlags;
    }

    public void setConverterFlags(int converterFlags) {
        this.converterFlags = converterFlags;
    }

    public int getJsonProtocolType() {
        return JsonProtocolType;
    }

    public void setJsonProtocolType(int jsonProtocolType) {
        JsonProtocolType = jsonProtocolType;
    }

    public int getMajorVersion() {
        return majorVersion;
    }

    public void setMajorVersion(int majorVersion) {
        this.majorVersion = majorVersion;
    }

    public int getMinorVersion() {
        return minorVersion;
    }

    public void setMinorVersion(int minorVersion) {
        this.minorVersion = minorVersion;
    }

    @Override
    public void clear() {
        converterFlags = 0;
        JsonProtocolType = JsonProtocol.JSON_JPT_UNKNOWN;
        majorVersion = 0;
        minorVersion = 0;
    }
}
