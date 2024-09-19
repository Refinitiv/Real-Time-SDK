/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

abstract class JsonNonRWFConverter extends AbstractContainerTypeConverter {

    JsonNonRWFConverter(JsonAbstractConverter converter) {
        super(converter);

    }

    @Override
    Object getContainerObject() {
        return JsonFactory.createBuffer();
    }

    @Override
    void releaseContainer(Object type) {
        JsonFactory.releaseBuffer((Buffer) type);
    }

    @Override
    protected int decodeEntry(DecodeIterator decIter, Object entryObj) {
        return CodecReturnCodes.FAILURE;
    }

    @Override
    protected boolean writeContent(DecodeIterator decIter, JsonBuffer outBuffer, JsonConverterError error, Object container) {
        return BasicPrimitiveConverter.writeAsciiString((Buffer) container, outBuffer, error);
    }

    @Override
    protected int decodeContainer(DecodeIterator decIter, Object setDb, Object container) {
        Buffer buff = (Buffer) container;
        buff.clear();
        return buff.decode(decIter);
    }

    @Override
    protected boolean writeEntry(DecodeIterator decIterator, JsonBuffer outBuffer, Object localSetDb, JsonConverterError error, Object entryObj, Object container) {
        return false;
    }

    @Override
    void encodeRWF(JsonNode node, String key, EncodeIterator iter, JsonConverterError error) {

        if (!node.isTextual()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected string value, found type " + node.getNodeType().toString());
            return;
        }

        Buffer xmlBuf = JsonFactory.createBuffer();
        Buffer dataBuf = JsonFactory.createBuffer();
        try {

            xmlBuf.clear();
            dataBuf.clear();
            converter.getPrimitiveHandler(DataTypes.ASCII_STRING).decodeJson(node, xmlBuf, error);
            if (error.isFailed())
                return;
            if (iter.encodeNonRWFInit(dataBuf) < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR, "Failed to initialize " + getContainerName(dataTypes[0]) + " encoding");
                return;
            }
            dataBuf.data().put(xmlBuf.data().array(), xmlBuf.position(), xmlBuf.length());
            if (iter.encodeNonRWFComplete(dataBuf, true) < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR, "Failed to complete " + getContainerName(dataTypes[0]) + " encoding");
                return;
            }
        } finally {
            JsonFactory.releaseBuffer(xmlBuf);
            JsonFactory.releaseBuffer(dataBuf);
        }

        return;
    }

}
