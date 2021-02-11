package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.UpdateMsg;

import java.util.Iterator;

import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonConfInfoConverter extends AbstractRsslMessageChunkTypeConverter {
    JsonConfInfoConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        checkObject(node, JSON_CONFINFO, error);
        if (error.isFailed())
            return;

        UpdateMsg updateMsg = (UpdateMsg) msg;

        boolean hasCount = false;
        boolean hasTime = false;

        for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
            String key = it.next();
            JsonNode currentNode = node.path(key);
            switch (key) {
                case JSON_COUNT:
                    updateMsg.conflationCount(getInt(currentNode, key, error));
                    hasCount = true;
                    break;

                case JSON_TIME:
                    updateMsg.conflationTime(getInt(currentNode, key, error));
                    hasTime = true;
                    break;

                default:
                    processUnexpectedKey(key, error);
            }

            if (error.isFailed())
                return;
        }

        if (!hasCount)
            error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, JSON_COUNT);

        if (!hasTime)
            error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, JSON_TIME);

    }
}
