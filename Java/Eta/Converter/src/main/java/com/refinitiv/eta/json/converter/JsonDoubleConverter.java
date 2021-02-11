package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.Double;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.json.util.JsonFactory;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonDoubleConverter extends AbstractPrimitiveTypeConverter {

    JsonDoubleConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] {DataTypes.DOUBLE, DataTypes.DOUBLE_8 };
    }

    @Override
    Object getPrimitiveType() {
        return JsonFactory.createDouble();
    }

    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseDouble((Double) type);
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        Double dv = (Double) type;
        dv.clear();
        return dv.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return BasicPrimitiveConverter.writeDouble((Double) type, outBuffer, false, error);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        Double dv = (Double) msg;
        dv.clear();
        if (node.isNull()) {
            dv.blank();
        } else {
            if (node.isDouble())
                dv.value(node.doubleValue());
            else if (node.isTextual()) {
                switch (node.textValue()) {
                    case jsonDoublePositiveInfinityStr:
                        dv.value(java.lang.Double.POSITIVE_INFINITY);
                        break;
                    case jsonDoubleNegativeInfinityStr:
                        dv.value(java.lang.Double.NEGATIVE_INFINITY);
                        break;
                    case jsonDoubleNanStr:
                        dv.value(java.lang.Double.NaN);
                        break;
                    default:
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_PARSE_ERROR, "Invalid text '" + node.textValue() + "' expected Double");
                        return;
                }
            }
        }
    }

    @Override
    protected void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {
        if (dataNode.isNull())
            return;
        Double dv = JsonFactory.createDouble();
        try {
            decodeJson(dataNode, dv, error);
            int result = dv.encode(iter);
            if (result != SUCCESS) {
                error.setEncodeError(result, key);
                return;
            }
        } finally {
            JsonFactory.releaseDouble(dv);
        }

    }
}
