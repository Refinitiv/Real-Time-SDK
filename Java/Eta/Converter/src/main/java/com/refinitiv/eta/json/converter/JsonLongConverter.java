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
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.json.util.JsonFactory;

import java.math.BigInteger;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;

class JsonLongConverter extends AbstractPrimitiveTypeConverter {

    public static final BigInteger MAX_BIG_INTEGER_VALUE = new BigInteger("18446744073709551615");

    JsonLongConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.UINT, DataTypes.UINT_1, DataTypes.UINT_2, DataTypes.UINT_4, DataTypes.UINT_8};
    }

    @Override
    Object getPrimitiveType() {
        return JsonFactory.createUInt();
    }

    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseUInt((UInt) type);
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        UInt uInt = (UInt) type;
        uInt.clear();
        return uInt.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        final long longValue = ((UInt) type).toLong();
        if(longValue < 0)
            return BasicPrimitiveConverter.writeBigInteger(((UInt) type).toBigInteger(), outBuffer, error);
        else
            return BasicPrimitiveConverter.writeLong(longValue, outBuffer, error);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {

        UInt iv = (UInt) msg;
        iv.clear();
        if (node.isNull()) {
            return;
        }
        if (node.isInt())
            iv.value(node.asInt());
        else if (node.isLong())
            iv.value(node.asLong());
        else if (node.isBigInteger())
            iv.value(node.bigIntegerValue());
        else {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "expected type UInt, found " + node.getNodeType().toString());
            return;
        }
    }

    @Override
    void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {
        if (dataNode.isNull())
            return;

        UInt encUIntValue = JsonFactory.createUInt();
        try {
            encUIntValue.clear();
            if (dataNode.isInt())
                encUIntValue.value(dataNode.asInt());
            else if (dataNode.isLong())
                encUIntValue.value(dataNode.asLong());
            else if (dataNode.isBigInteger())
                encUIntValue.value(dataNode.bigIntegerValue());
            else{
                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "expected type UInt, found " + dataNode.getNodeType().toString());
                return;
            }
            int result = encUIntValue.encode(iter);
            if (result != SUCCESS) {
                error.setEncodeError(result, key);
                return;
            }
        } finally {
            JsonFactory.releaseUInt(encUIntValue);
        }
    }

    @Override
    boolean isInRange(int dataType, JsonNode dataNode) {
        if (dataNode.isDouble() || dataNode.isFloat()) {
            return false;
        }
        if (dataNode.isBigInteger()) {
            if (dataType == DataTypes.UINT || dataType == DataTypes.UINT_8) {
                BigInteger bigValue = dataNode.bigIntegerValue();
                return bigValue.compareTo(MAX_BIG_INTEGER_VALUE) <= 0;
            }
            return false;
        }

        long value = dataNode.asLong();
        switch (dataType) {
            case (DataTypes.UINT):
            case (DataTypes.UINT_8):
                if (value >= 0 && value <= Long.MAX_VALUE) {
                    return true;
                }
                break;

            case (DataTypes.UINT_1):
                if (value >= 0 && value <= 255) {
                    return true;
                }
                break;

            case (DataTypes.UINT_2):
                if (value >= 0 && value <= 65535) {
                    return true;
                }
                break;

            case (DataTypes.UINT_4):
                if (value >= 0 && value <= 4294967295L) {
                    return true;
                }
                break;
            default:
                return false;
        }
        return false;
    }
}
