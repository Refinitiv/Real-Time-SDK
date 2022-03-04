/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.json.util.JsonFactory;

import java.nio.ByteBuffer;
import java.util.Base64;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;

class JsonBufferConverter extends AbstractPrimitiveTypeConverter {

    JsonBufferConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.BUFFER };
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
        Buffer buffer = (Buffer) type;
        buffer.clear();
        return buffer.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return BufferConverter.writeToJson((Buffer) type, outBuffer, error);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        Buffer inBuffer = (Buffer) msg;
        inBuffer.clear();
        if (node.isNull()) {
            return;
        }
        try {
            byte[] bytes = Base64.getDecoder().decode(node.textValue());
            inBuffer.data(ByteBuffer.wrap(bytes));
        } catch (Exception e) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Failed decoding buffer: " + node.asText());
        }
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
