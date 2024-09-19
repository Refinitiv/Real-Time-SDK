/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.node.JsonNodeType;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

import java.nio.ByteBuffer;
import java.util.Iterator;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonArrayConverter extends AbstractPrimitiveTypeConverter {

    JsonArrayConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.ARRAY };
    }

    @Override
    boolean encodeJson(DecodeIterator decIter, JsonBuffer outBuffer, JsonConverterError error) {

        Array array = JsonFactory.createArray();
        ArrayEntry arrayEntry = JsonFactory.createArrayEntry();
        try {

            AbstractPrimitiveTypeConverter conv = null;

            array.clear();
            arrayEntry.clear();

            int ret = array.decode(decIter);

            if (ret < CodecReturnCodes.SUCCESS){
                error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, null);
                return false;
            }

            if (ret == CodecReturnCodes.BLANK_DATA)
                return BufferHelper.writeArray(ConstCharArrays.nullString, outBuffer, false, error);

            BufferHelper.beginObject(outBuffer, error);
            BufferHelper.writeArrayAndColon(JSON_TYPE, outBuffer, false, error);
            BufferHelper.writeArray(getDataType(array.primitiveType()), outBuffer, true, error);
            BufferHelper.writeArrayAndColon(JSON_DATA, outBuffer, true, error);
            BufferHelper.beginArray(outBuffer, error);

            boolean writeComma = false;

            conv = converter.getPrimitiveHandler(array.primitiveType());
            if (conv != null) {
                while (arrayEntry.decode(decIter) != CodecReturnCodes.END_OF_CONTAINER) {
                    if (!writeComma)
                        writeComma = true;
                    else
                        BufferHelper.comma(outBuffer, error);

                    if (!conv.encodeJson(decIter, outBuffer, error))
                        return false;
                }
            } else {
                error.setError(JsonConverterErrorCodes.JSON_ERROR, "Unknown primitive type: " + array.primitiveType());
                return false;
            }

            BufferHelper.endArray(outBuffer, error);
            BufferHelper.endObject(outBuffer, error);
            return error.isSuccessful();

        } catch (Exception e) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR, e.getMessage());
            return false;
        } finally {
            JsonFactory.releaseArray(array);
            JsonFactory.releaseArrayEntry(arrayEntry);
        }
    }


    @Override
    int decode(DecodeIterator decIter, Object type) {
        return 0;
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return false;
    }

    @Override
    boolean encodeJson(Buffer buffer, JsonBuffer outBuffer, JsonConverterError error) {
        return false;
    }

    @Override
    protected void encodeRWF(JsonNode currentNode, String key, EncodeIterator iter, JsonConverterError error) {

        if (currentNode.isNull()) {
            Array array = JsonFactory.createArray();
            try {
                array.clear();
            } finally {
                JsonFactory.releaseArray(array);
            }
            return;
        }


        if (!currentNode.isObject()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected object type, found " + currentNode.getNodeType().toString(), key);
            return;
        }

        Array array = JsonFactory.createArray();
        ArrayEntry arrayEntry = null;
        array.clear();
        boolean hasType = false;
        JsonNode data = null;
        JsonNodeType dataType = null;
        EncodeIterator arrayEncodeIterator = null;
        Buffer localBuffer = null;
        ByteBuffer byteBuffer = null;

        try {
            for (Iterator<String> it = currentNode.fieldNames(); it.hasNext(); ) {
                String next = it.next();
                JsonNode child = currentNode.path(next);
                switch (next) {
                    case JSON_LENGTH:
                        if (!child.isInt()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected integer type for LENGTH, found " + currentNode.getNodeType().toString(), key);
                            return;
                        }
                        array.itemLength(child.asInt());
                        break;
                    case JSON_TYPE:
                        hasType = true;
                        int type = converter.getDataType(child);
                        if (type == CodecReturnCodes.FAILURE) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected primitiveType, found " + child.asText(), key);
                            return;
                        } else if (type > DataTypes.BASE_PRIMITIVE_MAX) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_PRIMITIVE_TYPE, "Expected base primitiveType, found " + child.asText(), key);
                            return;
                        }
                        array.primitiveType(type);
                        break;
                    case JSON_DATA:

                        if (!child.isArray() && !child.isNull()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected array, found " + child.getNodeType(), key);
                            return;
                        }
                        data = child;
                        break;
                    default:
                        if (converter.catchUnexpectedKeys()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, "key = " + next);
                            return;
                        }
                }
            }

            if (data.isMissingNode()) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, "Missing Data token", key);
                return;
            }

            if (!hasType) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, "Missing Type token", key);
                return;
            }

            int ret = array.encodeInit(iter);
            if (ret < SUCCESS) {
                return;
            }


            if (currentNode.size() > 0) {
                arrayEncodeIterator = JsonFactory.createEncodeIterator();
                localBuffer = JsonFactory.createBuffer();
                byteBuffer = JsonFactory.createByteBuffer();
                localBuffer.data(byteBuffer);
                arrayEntry = JsonFactory.createArrayEntry();
                arrayEncodeIterator.setBufferAndRWFVersion(localBuffer, Codec.majorVersion(), Codec.minorVersion());
            }


            if (data != null) {
                for (int i = 0; i < data.size(); i++) {

                    if (i == 0) {
                        dataType = data.get(i).getNodeType();
                    }

                    if (!dataType.equals(data.get(i).getNodeType())) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_TYPE_MISMATCH, "Array entry types mismatch: "
                                + dataType.toString() + ", " + data.get(i).getNodeType().toString(), key);
                        return;
                    }

                    arrayEntry.clear();

                    converter.decodeChunk(array.primitiveType(), data.get(i), key, arrayEncodeIterator, error);
                    if (error.isFailed())
                        break;

                    arrayEntry.encodedData(localBuffer);
                    ret = arrayEntry.encode(iter);

                    if (ret != SUCCESS) {
                        error.setEncodeError(ret, "ARRAY element [" + i + "]");
                        break;
                    }

                    byteBuffer.clear();
                    localBuffer.data(byteBuffer);
                    arrayEncodeIterator.clear();
                    arrayEncodeIterator.setBufferAndRWFVersion(localBuffer, Codec.majorVersion(), Codec.minorVersion());
                }
            }

            ret = array.encodeComplete(iter, error.isSuccessful() && (ret >= SUCCESS));
            if (error.isFailed())
                return;

            if (ret != SUCCESS) {
                error.setEncodeError(ret, "ARRAY encode complete");
                return;
            }
        } finally {
            JsonFactory.releaseArray(array);
            if (arrayEntry != null)
                JsonFactory.releaseArrayEntry(arrayEntry);
            if (localBuffer != null)
                JsonFactory.releaseBuffer(localBuffer);
            if (byteBuffer != null)
                JsonFactory.releaseByteBuffer(byteBuffer);
            if (arrayEncodeIterator != null)
                JsonFactory.releaseEncodeIterator(arrayEncodeIterator);

        }
    }

    private String getDataType(int dataType) {

        if (dataType >= DataTypes.UNKNOWN && dataType <= DataTypes.RMTES_STRING)
            return ConstCharArrays.dataTypeStrings[dataType];
        else if (dataType >= DataTypes.INT_1 && dataType <= DataTypes.TIME_8)
            return ConstCharArrays.dataTypeStrings[dataType - 44];
        else if (dataType >= DataTypes.OPAQUE && dataType <= DataTypes.CONTAINER_TYPE_MAX)
            return ConstCharArrays.dataTypeStrings[dataType - 88];
        else if (dataType == DataTypes.NO_DATA)
            return ConstCharArrays.dataTypeStrings[41];
        return null;
    }
}
