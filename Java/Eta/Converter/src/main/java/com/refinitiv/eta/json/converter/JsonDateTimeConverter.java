package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DateTime;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.json.util.JsonFactory;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;

class JsonDateTimeConverter extends AbstractPrimitiveTypeConverter {

    JsonDateTimeConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] {DataTypes.DATETIME, DataTypes.DATETIME_7, DataTypes.DATETIME_9, DataTypes.DATETIME_11, DataTypes.DATETIME_12 };
    }

    @Override
    Object getPrimitiveType() {
        return JsonFactory.createDateTime();
    }

    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseDateTime((DateTime) type);
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        DateTime dt = (DateTime) type;
        dt.clear();
        return dt.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return BasicPrimitiveConverter.writeDateTime((DateTime) type, outBuffer, error);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        DateTime dateTime = (DateTime) msg;
        dateTime.clear();
        if (node.isNull()) {
            dateTime.blank();
        } else {
            int ret = dateTime.value(node.textValue());
            if (ret < SUCCESS)
                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Invalid DateTime value: " + node.textValue() + ", code = " + ret);
        }
    }

    @Override
    protected void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {
        if (dataNode.isNull())
            return;
        DateTime dateTime = JsonFactory.createDateTime();
        try {
            decodeJson(dataNode, dateTime, error);
            int result = dateTime.encode(iter);
            if (result != SUCCESS) {
                error.setEncodeError(result, key);
                return;
            }
        } finally {
            JsonFactory.releaseDateTime(dateTime);
        }
    }
}
