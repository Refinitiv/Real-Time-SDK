/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

class JsonJsonConverter extends AbstractContainerTypeConverter {

    JsonJsonConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.JSON };
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
            return BufferHelper.writeArray(ConstCharArrays.nullBytes, outBuffer, false, error);
        try {
            converter.getMapper().readTree(buf.data().array(), buf.position(), buf.length());
        } catch (Exception e) {
            return false;
        }

        return BufferHelper.copyToByteArray(buf.data().array(), buf.position(), buf.length(), outBuffer, error);
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

        String json = node.toString();
        Buffer buffer = JsonFactory.createBuffer();
        buffer.clear();

        try {
            iter.encodeNonRWFInit(buffer);
            buffer.data().put(json.getBytes());

            if (iter.encodeNonRWFComplete(buffer, true) < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR, "Encoding failed.");
                return;
            }
        } finally {
            JsonFactory.releaseBuffer(buffer);
        }

        return;
    }
}
