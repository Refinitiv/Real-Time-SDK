/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.json.util.JsonFactory;

import java.util.Objects;

abstract class AbstractPrimitiveTypeConverter extends AbstractTypeConverter {

    AbstractPrimitiveTypeConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    boolean encodeJson(Buffer buffer, JsonBuffer outBuffer, JsonConverterError error) {

        if (Objects.isNull(buffer) || Objects.isNull(buffer.data())) {
            return false;
        }

        DecodeIterator decIter = JsonFactory.createDecodeIterator();
        try {
            decIter.clear();
            decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
            return encodeJson(decIter, outBuffer, error);
        } finally {
            JsonFactory.releaseDecodeIterator(decIter);
        }
    }

    boolean encodeJson(DecodeIterator decIter, JsonBuffer outBuffer, JsonConverterError error) {

        Object type = getPrimitiveType();

        try {

            int ret = decode(decIter, type);

            if (ret < CodecReturnCodes.SUCCESS){
                error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, null);
                return false;
            }
            boolean res;
            if (ret == CodecReturnCodes.BLANK_DATA)
                res = BufferHelper.writeArray(ConstCharArrays.nullString, outBuffer, false, error);
            else
                res = writeToJson(outBuffer, type, error);
            if (!res)
                error.setError(JsonConverterErrorCodes.JSON_ERROR_OUT_OF_MEMORY, null);

            return res;

        } catch (Exception e) {

            error.setError(JsonConverterErrorCodes.JSON_ERROR, e.getMessage());
            return false;
        } finally {

            releasePrimitiveType(type);
        }
    }

    Object getPrimitiveType() { return null; }
    void releasePrimitiveType(Object type) {}
    boolean isInRange(int dataType, JsonNode dataNode) { return true;}

    abstract int decode(DecodeIterator decIter, Object type);
    abstract boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error);
}
