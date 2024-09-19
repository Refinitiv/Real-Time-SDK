/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;

public class JsonUnknownTypeConverter extends AbstractPrimitiveTypeConverter {

    JsonUnknownTypeConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.UNKNOWN };
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        return 0;
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return false;
    }

    @Override
    boolean encodeJson(DecodeIterator decIter, JsonBuffer outBuffer, JsonConverterError error) {
        error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_PRIMITIVE_TYPE, "Unknown type is not supported.");
        return false;
    }

    @Override
    void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {
        converter.getPrimitiveHandler(DataTypes.BUFFER).encodeRWF(dataNode, key, iter, error);
    }

    @Override
    public void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_PRIMITIVE_TYPE, "Unknown type not supported");
    }

}
