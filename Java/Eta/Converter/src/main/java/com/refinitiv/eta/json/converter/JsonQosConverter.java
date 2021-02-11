package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

import java.util.Iterator;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonQosConverter extends AbstractPrimitiveTypeConverter {

    JsonQosConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.QOS };
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        Qos qos = (Qos) msg;

        if (node.isNull()) {
            qos.clear();
            return;
        }

        checkObject(node, JSON_QOS, error);
        if (error.isFailed())
            return;

        for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
            String key = it.next();
            JsonNode currentNode = node.path(key);
            int result = SUCCESS;
            switch (key) {
                case JSON_TIMELINESS:
                    result = decodeTimeliness(currentNode, key, qos, error);
                    break;
                case JSON_RATE:
                    result = decodeRate(currentNode, key, qos, error);
                    break;
                case JSON_TIMEINFO:
                    qos.timeInfo(getInt(currentNode, key, error));
                    break;
                case JSON_RATEINFO:
                    qos.rateInfo(getInt(currentNode, key, error));
                    break;
                default:
                    processUnexpectedKey(key, error);
            }

            if (error.isFailed())
                return;

            if (result != SUCCESS)
                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, key + "=" + currentNode.asText());

        }
    }

    private int decodeRate(JsonNode currentNode, String key, Qos qos, JsonConverterError error) {
        checkStringOrInt(currentNode, key, error);
        if (error.isFailed())
            return CodecReturnCodes.FAILURE;

        if (currentNode.isTextual())
            return qos.rate(ConstCharArrays.JsonQosRate.ofValue(getText(currentNode, key, error), error));
        else
            return qos.rate(getInt(currentNode, key, error));
    }

    private int decodeTimeliness(JsonNode currentNode, String key, Qos qos, JsonConverterError error) {
        checkStringOrInt(currentNode, key, error);
        if (error.isFailed())
            return CodecReturnCodes.FAILURE;

        if (currentNode.isTextual())
            return qos.timeliness(ConstCharArrays.JsonQosTimeliness.ofValue(getText(currentNode, key, error), error));
        else
            return qos.timeliness(getInt(currentNode, key, error));
    }

    @Override
    Object getPrimitiveType() {
        return JsonFactory.createQos();
    }
    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseQos((Qos) type);
    }


    @Override
    int decode(DecodeIterator decIter, Object type) {
        Qos qos = (Qos) type;
        qos.clear();
        return qos.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return writeToJson((Qos) type, outBuffer, error);
    }

    boolean writeToJson(Qos qos, JsonBuffer outBuffer, JsonConverterError error) {

        BufferHelper.beginObject(outBuffer, error);
        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_TIMELINESS, outBuffer, false, error);
        BufferHelper.writeArray(getQosTimeliness(qos.timeliness()), outBuffer, true, error);
        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_RATE, outBuffer, true, error);
        BufferHelper.writeArray(getQosRate(qos.rate()), outBuffer, true, error);

        if (qos.isDynamic()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_DYNAMIC, outBuffer, true, error);
            BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
        }
        if (qos.timeliness() == QosTimeliness.DELAYED) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_TIMEINFO, outBuffer, true, error);
            BasicPrimitiveConverter.writeLong(qos.timeInfo(), outBuffer, error);
        }
        if (qos.rate() == QosRates.TIME_CONFLATED) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_RATEINFO, outBuffer, true, error);
            BasicPrimitiveConverter.writeLong(qos.rateInfo(), outBuffer, error);
        }

        return BufferHelper.endObject(outBuffer, error);
    }

    private static String getQosTimeliness(int time) {
        if (QosTimeliness.UNSPECIFIED <= time && time <= QosTimeliness.DELAYED)
            return qosTimelinessStrings[time];
        return null;
    }

    private static String getQosRate(int rate) {
        if (QosRates.UNSPECIFIED <= rate && rate <= QosRates.TIME_CONFLATED)
            return qosRateStrings[rate];
        return null;
    }

    @Override
    void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {
        if (dataNode.isNull())
            return;

        Qos encQosValue = JsonFactory.createQos();

        try {
            if(!dataNode.path(JSON_TIMELINESS).isMissingNode())
                decodeTimeliness(dataNode.path(JSON_TIMELINESS), JSON_TIMELINESS, encQosValue, error);

            if(!dataNode.path(JSON_RATE).isMissingNode())
                decodeRate(dataNode.path(JSON_RATE), JSON_RATE, encQosValue, error);

            if(!dataNode.path(JSON_TIMEINFO).isMissingNode())
                encQosValue.timeInfo(getInt(dataNode.path(JSON_TIMEINFO), key, error));

            if(!dataNode.path(JSON_RATEINFO).isMissingNode())
                encQosValue.rateInfo(getInt(dataNode.path(JSON_RATEINFO), key, error));

            int result = encQosValue.encode(iter);
            if (result != SUCCESS) {
                error.setEncodeError(result, key);
                return;
            }
        } finally {
            JsonFactory.releaseQos(encQosValue);
        }
    }
}
