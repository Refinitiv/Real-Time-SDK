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

class JsonRMTESConverter extends AbstractPrimitiveTypeConverter {

    JsonRMTESConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.RMTES_STRING };
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
        return BasicPrimitiveConverter.writeRMTESString((Buffer) type, outBuffer, error);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {

        Buffer buff = (Buffer) msg;
        if (node.isNull()) {
            buff.clear();
            buff.data(ByteBuffer.allocate(0));
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
        if (dataNode.isNull())
            return;

        Buffer encBufferValue = JsonFactory.createBuffer();
        try {
            encBufferValue.data(ByteBuffer.wrap(dataNode.textValue().getBytes(StandardCharsets.UTF_8)));

            int result = encBufferValue.encode(iter);
            if (result != SUCCESS)
                error.setEncodeError(result, key);
        } finally {
            JsonFactory.releaseBuffer(encBufferValue);
        }
    }
}
