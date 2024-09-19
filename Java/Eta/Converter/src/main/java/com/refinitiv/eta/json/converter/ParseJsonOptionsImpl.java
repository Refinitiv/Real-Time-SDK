/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

class ParseJsonOptionsImpl implements ParseJsonOptions {

    private int converterFlags;
    private int protocolType;

    ParseJsonOptionsImpl() {}

    @Override
    public void clear() {
        converterFlags = ParseJsonFlags.JSON_PJF_NONE;
        protocolType = JsonProtocol.JSON_JPT_UNKNOWN;
    }

    public int getConverterFlags() {
        return converterFlags;
    }

    public void setConverterFlags(int converterFlags) {
        this.converterFlags = converterFlags;
    }

    public int getProtocolType() {
        return protocolType;
    }

    public void setProtocolType(int protocolType) {
        this.protocolType = protocolType;
    }
}
