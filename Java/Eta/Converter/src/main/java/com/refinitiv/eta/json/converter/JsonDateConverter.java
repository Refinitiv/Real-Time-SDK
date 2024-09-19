/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.Date;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.json.util.JsonFactory;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;

class JsonDateConverter extends AbstractPrimitiveTypeConverter {

    JsonDateConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.DATE, DataTypes.DATE_4 };
    }

    @Override
    Object getPrimitiveType() {
        return JsonFactory.createDate();
    }

    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseDate((Date) type);
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        Date date = (Date) type;
        date.clear();
        return date.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return BasicPrimitiveConverter.writeDate((Date) type, outBuffer, error);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        Date date = (Date) msg;
        date.clear();
        if (node.isNull()) {
            date.blank();
        } else {
            int ret = date.value(node.textValue());
            if (ret < SUCCESS)
                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Invalid date value: " + node.textValue() + ", code = " + ret);
        }
    }

    @Override
    protected void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {
        if (dataNode.isNull())
            return;
        Date date = JsonFactory.createDate();
        try {
            decodeJson(dataNode, date, error);
            int result = date.encode(iter);
            if (result != SUCCESS) {
                error.setEncodeError(result, key);
                return;
            }
        } finally {
            JsonFactory.releaseDate(date);
        }

    }
}
