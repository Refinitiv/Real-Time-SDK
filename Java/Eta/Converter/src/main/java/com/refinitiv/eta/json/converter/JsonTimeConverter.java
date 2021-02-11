package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Time;
import com.refinitiv.eta.json.util.JsonFactory;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;

class JsonTimeConverter extends AbstractPrimitiveTypeConverter {

    JsonTimeConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] {DataTypes.TIME, DataTypes.TIME_3, DataTypes.TIME_5, DataTypes.TIME_7, DataTypes.TIME_8 };
    }

    @Override
    Object getPrimitiveType() {
        return JsonFactory.createTime();
    }

    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseTime((Time) type);
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        Time time = (Time) type;
        time.clear();
        return time.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return BasicPrimitiveConverter.writeTime((Time) type, outBuffer, error);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        Time time = (Time) msg;
        time.clear();
        if (node.isNull()) {
            time.blank();
            return;
        }
        int ret = time.value(node.textValue());
        if (ret < SUCCESS)
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Invalid time value: " + node.textValue() + ", code = " + ret);
    }

    @Override
    protected void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {
        if (dataNode.isNull())
            return;
        Time time = JsonFactory.createTime();
        try {
            time.clear();
            decodeJson(dataNode, time, error);
            int result = time.encode(iter);
            if (result != SUCCESS) {
                error.setEncodeError(result, key);
                return;
            }
        } finally {
            JsonFactory.releaseTime(time);
        }
    }
}
