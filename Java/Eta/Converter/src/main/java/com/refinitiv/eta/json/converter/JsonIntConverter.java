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
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.json.util.JsonFactory;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;

class JsonIntConverter extends AbstractPrimitiveTypeConverter {

    JsonIntConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.INT, DataTypes.INT_1, DataTypes.INT_2, DataTypes.INT_4, DataTypes.INT_8};
    }

    @Override
    Object getPrimitiveType() {
        return JsonFactory.createInt();
    }
    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseInt((Int) type);
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        Int iv = (Int) type;
        iv.clear();
        return iv.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return BasicPrimitiveConverter.writeLong(((Int) type).toLong(), outBuffer, error);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {

        Int iv = (Int) msg;
        iv.clear();
        if (node.isNull()) {
            return;
        }
        if (node.isInt())
            iv.value(node.asInt());
        else if (node.isLong())
            iv.value(node.longValue());
        else {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "expected integer type, found " + node.getNodeType().toString());
            return;
        }
    }

    @Override
    protected void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {
        if (dataNode.isNull())
            return;

        Int encIntValue = JsonFactory.createInt();
        try {
            encIntValue.clear();
            if (dataNode.isInt())
                encIntValue.value(dataNode.asInt());
            else if (dataNode.isLong())
                encIntValue.value(dataNode.longValue());
            else {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "expected integer type, found " + dataNode.getNodeType().toString());
                return;
            }
            int result = encIntValue.encode(iter);
            if (result != SUCCESS) {
                error.setEncodeError(result, key);
                return;
            }
        } finally {
            JsonFactory.releaseInt(encIntValue);
        }
    }

    @Override
    boolean isInRange(int dataType, JsonNode dataNode) {
        if (dataNode.isBigInteger() ||
            dataNode.isDouble() ||
            dataNode.isFloat()) {
            return false;
        }
        long value = dataNode.asLong();
        switch (dataType) {
            case (DataTypes.INT):
            case (DataTypes.INT_8):
                if (value >= Long.MIN_VALUE && value <= Long.MAX_VALUE) {
                    return true;
                }
                break;

            case (DataTypes.INT_1):
                if (value >= Byte.MIN_VALUE && value <= Byte.MAX_VALUE) {
                    return true;
                }
                break;

            case (DataTypes.INT_2):
                if (value >= Short.MIN_VALUE && value <= Short.MAX_VALUE) {
                    return true;
                }
                break;

            case (DataTypes.INT_4):
                if (value >= Integer.MIN_VALUE && value <= Integer.MAX_VALUE) {
                    return true;
                }
                break;
            default:
                return false;
        }
        return false;
    }
}
