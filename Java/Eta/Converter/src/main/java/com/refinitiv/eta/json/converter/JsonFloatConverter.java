/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Float;
import com.refinitiv.eta.json.util.JsonFactory;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonFloatConverter extends AbstractPrimitiveTypeConverter {

    JsonFloatConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] {DataTypes.FLOAT, DataTypes.FLOAT_4 };
    }

    @Override
    Object getPrimitiveType() {
        return JsonFactory.createFloat();
    }

    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseFloat((Float) type);
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        Float fv = (Float) type;
        fv.clear();
        return fv.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return BasicPrimitiveConverter.writeFloat((Float) type, outBuffer, false, error);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {

        Float fv = (Float) msg;
        fv.clear();

        if (node.isNull()) {
            fv.blank();
            return;
        }


        if (node.isFloat() || node.isDouble())
            fv.value((float)node.asDouble());
        else if (node.isTextual()) {
            switch (node.textValue()) {
                case jsonDoublePositiveInfinityStr:
                    fv.value(java.lang.Float.POSITIVE_INFINITY);
                    break;
                case jsonDoubleNegativeInfinityStr:
                    fv.value(java.lang.Float.NEGATIVE_INFINITY);
                    break;
                case jsonDoubleNanStr:
                    fv.value(java.lang.Float.NaN);
                    break;
                default:
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_PARSE_ERROR, "Invalid text '" + node.textValue() + "' expected Float");
                    return;
            }
        }
    }

    @Override
    protected void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {
        if (dataNode.isNull())
            return;

        Float encFloatValue = JsonFactory.createFloat();

        try {
            decodeJson(dataNode, encFloatValue, error);

            int result = encFloatValue.encode(iter);
            if (result != SUCCESS) {
                error.setEncodeError(result, key);
                return;
            }
        } finally {
            JsonFactory.releaseFloat(encFloatValue);
        }
    }
}
