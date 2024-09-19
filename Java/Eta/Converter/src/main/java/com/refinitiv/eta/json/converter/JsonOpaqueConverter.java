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

class JsonOpaqueConverter extends AbstractContainerTypeConverter {

    JsonOpaqueConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.OPAQUE };
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

        Buffer buf = (Buffer) container;
        if (buf.length() == 0)
            return BufferHelper.writeArray(ConstCharArrays.nullString, outBuffer, false, error);
        else
            return BufferConverter.writeToJson(buf, outBuffer, error);
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
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected string value.");
            return;
        }

        Buffer opaqueBuf = JsonFactory.createBuffer();
        Buffer dataBuf = JsonFactory.createBuffer();

        try {

            opaqueBuf.clear();
            dataBuf.clear();
            converter.getPrimitiveHandler(DataTypes.BUFFER).decodeJson(node, opaqueBuf, error);
            if (error.isFailed())
                return;
            if (iter.encodeNonRWFInit(dataBuf) < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR, "Encoding failed");
                return;
            }
            dataBuf.data().put(opaqueBuf.data().array(), opaqueBuf.position(), opaqueBuf.length());
            if (iter.encodeNonRWFComplete(dataBuf, true) < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR, "Failed to complete Opaque encoding to RWF");
            }
        } finally {
            JsonFactory.releaseBuffer(opaqueBuf);
            JsonFactory.releaseBuffer(dataBuf);
        }

        return;
    }

}
