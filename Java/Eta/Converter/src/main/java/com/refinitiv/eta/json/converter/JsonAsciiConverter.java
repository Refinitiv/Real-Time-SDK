/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.json.util.JsonFactory;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;

class JsonAsciiConverter extends AbstractPrimitiveTypeConverter {

    JsonAsciiConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.ASCII_STRING };
    }

    @Override
    Object getPrimitiveType() {
        return JsonFactory.createBuffer();
    }

    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseBuffer((Buffer) type);
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        Buffer buf = (Buffer) type;
        buf.clear();
        return buf.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return BasicPrimitiveConverter.writeAsciiString(((Buffer) type), outBuffer, error);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {

        Buffer buff = (Buffer) msg;

        if (node.isNull()) {
            buff.clear();
            return;
        }
        if (!node.isTextual()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected string value");
            return;
        }

        buff.clear();
        buff.data(node.asText());
        return;
    }

    @Override
    protected void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {

        Buffer encBufferValue = JsonFactory.createBuffer();
        try {
            decodeJson(dataNode, encBufferValue, error);
            if (error.isSuccessful()) {
                int result = encBufferValue.encode(iter);
                if (result != SUCCESS)
                    error.setEncodeError(result, key);
            }
        } finally {
            JsonFactory.releaseBuffer(encBufferValue);
        }
    }
}
