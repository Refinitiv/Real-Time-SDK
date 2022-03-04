/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.RequestMsg;

import java.util.Iterator;

import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonPriorityConverter extends AbstractRsslMessageChunkTypeConverter {
    JsonPriorityConverter(JsonAbstractConverter converter) {
        super(converter);
    }


    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        checkObject(node, JSON_PRIORITY, error);
        if (error.isFailed())
            return;

        RequestMsg requestMsg = (RequestMsg) msg;

        boolean hasClass = false;
        boolean hasCount = false;

        for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
            String key = it.next();
            JsonNode currentNode = node.path(key);
            switch(key) {
                case JSON_CLASS:
                    requestMsg.priority().priorityClass(getInt(currentNode, key, error));
                    hasClass = true;
                    break;
                case JSON_COUNT:
                    requestMsg.priority().count(getInt(currentNode, key, error));
                    hasCount = true;
                    break;
                default:
                    processUnexpectedKey(key, error);
            }

            if (error.isFailed())
                return;
        }

        if (!hasClass) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, JSON_PRIORITY + "." + JSON_CLASS);
            return;
        }

        if (!hasCount) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, JSON_PRIORITY + "." + JSON_COUNT);
            return;
        }
    }
}
