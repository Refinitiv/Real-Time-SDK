/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
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
import java.nio.charset.StandardCharsets;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;

class JsonUtf8Converter extends AbstractPrimitiveTypeConverter {

    JsonUtf8Converter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.UTF8_STRING };
    }

    @Override
    Object getPrimitiveType() { return JsonFactory.createBuffer(); }
    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseBuffer((Buffer) type);
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        Buffer buffer = (Buffer) type;
        return buffer.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return BasicPrimitiveConverter.writeUTF8String((Buffer) type, outBuffer, error);
    }

    @Override
    public void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        Buffer buffer = (Buffer) msg;
        if (node.isNull()) {
            buffer.clear();
            return;
        }

        buffer.data(ByteBuffer.wrap(node.textValue().getBytes(StandardCharsets.UTF_8)));
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
