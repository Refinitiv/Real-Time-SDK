/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

import java.util.Iterator;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonStateConverter extends AbstractPrimitiveTypeConverter {

    JsonStateConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.STATE };
    }

    @Override
    Object getPrimitiveType() {
        return JsonFactory.createState();
    }

    @Override
    void releasePrimitiveType(Object type) {
        JsonFactory.releaseState((State) type);
    }

    @Override
    int decode(DecodeIterator decIter, Object type) {
        State state = (State) type;
        state.clear();
        return state.decode(decIter);
    }

    @Override
    boolean writeToJson(JsonBuffer outBuffer, Object type, JsonConverterError error) {
        return writeToJson((State) type, outBuffer, error);
    }

    boolean writeToJson(State state, JsonBuffer outBuffer, JsonConverterError error) {

        BufferHelper.beginObject(outBuffer, error);
        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_STREAM, outBuffer, false, error);
        BufferHelper.writeArray(getStreamState(state.streamState()), outBuffer, true, error);
        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_DATA, outBuffer, true, error);
        BufferHelper.writeArray(getDataState(state.dataState()), outBuffer, true, error);

        if (state.code() > StateCodes.NONE) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_CODE, outBuffer, true, error);
            BufferHelper.writeArray(getStateCode(state.code()), outBuffer, true, error);
        }

        if (!state.text().isBlank()) {
            BufferHelper.comma(outBuffer, error);
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_TEXT, outBuffer, false, error);
            BufferHelper.doubleQuote(outBuffer, error);

            int i = state.text().position();
            while (i < state.text().length() + state.text().position()) {
                switch (state.text().data().get(i)) {
                    case '\"':
                        BufferHelper.writeArray(ConstCharArrays.safeQuote, outBuffer, false, error);
                        break;
                    case '\\':
                        BufferHelper.writeArray(ConstCharArrays.safeBackslash, outBuffer, false, error);
                        break;
                    default:
                        if (state.text().data().get(i) < (' ' & 0xFF) || state.text().data().get(i) == 0x7F) {
                            BufferHelper.writeCharAsHex0(state.text().data().get(i), outBuffer, error);
                        } else
                            BufferHelper.writeByte(outBuffer, state.text().data().get(i), error);
                }
                i++;
            }

            BufferHelper.doubleQuote(outBuffer, error);
        }

        return BufferHelper.endObject(outBuffer, error);
    }

    private static String getStateCode(int stateCode) {
        if (StateCodes.NONE <= stateCode && stateCode <= StateCodes.NOT_OPEN
                || StateCodes.NON_UPDATING_ITEM <= stateCode && stateCode <= StateCodes.UNABLE_TO_REQUEST_AS_BATCH
                || StateCodes.NO_BATCH_VIEW_SUPPORT_IN_REQ <= stateCode && stateCode <= StateCodes.DACS_USER_ACCESS_TO_APP_DENIED
                || stateCode == StateCodes.GAP_FILL || stateCode == StateCodes.APP_AUTHORIZATION_FAILED) {
            return stateCodesString[stateCode];
        }
        return null;
    }

    private static String getDataState(int dataState) {
        if (DataStates.NO_CHANGE <= dataState && dataState <= DataStates.SUSPECT)
            return dataStateStrings[dataState];
        return null;
    }

    private static String getStreamState(int streamState) {
        if (StreamStates.UNSPECIFIED <= streamState && streamState <= StreamStates.REDIRECTED)
            return streamStateStrings[streamState];
        return null;
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        checkObject(node, JSON_STATE, error);
        if (error.isFailed())
            return;

        State state = (State) msg;
        Buffer textBuffer = JsonFactory.createBuffer();

        try {
            for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
                String key = it.next();
                JsonNode currentNode = node.path(key);
                int result = SUCCESS;
                switch (key) {
                    case JSON_CODE:
                        result = decodeStateCode(currentNode, key, state, error);
                        break;
                    case JSON_DATA:
                        result = decodeDataState(currentNode, key, state, error);
                        break;

                    case JSON_STREAM:
                        result = decodeStreamState(currentNode, key, state, error);
                        break;

                    case JSON_TEXT:
                        textBuffer.data(getText(currentNode, key, error));
                        state.text(textBuffer);
                        break;

                    default:
                        processUnexpectedKey(key, error);
                }

                if (error.isFailed())
                    return;

                if (result != SUCCESS)
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, key + "=" + currentNode.asText());

            }
        } finally {
            JsonFactory.releaseBuffer(textBuffer);
        }


    }

    private int decodeStreamState(JsonNode currentNode, String key, State state, JsonConverterError error) {
        checkStringOrInt(currentNode, key, error);
        if (error.isFailed())
            return CodecReturnCodes.FAILURE;

        if (currentNode.isTextual())
            return state.streamState(ConstCharArrays.JsonStreamState.ofValue(getText(currentNode, key, error), error));
        else
            return state.streamState(getInt(currentNode, key, error));
    }

    private int decodeDataState(JsonNode currentNode, String key, State state, JsonConverterError error) {
        checkStringOrInt(currentNode, key, error);
        if (error.isFailed())
            return CodecReturnCodes.FAILURE;

        if (currentNode.isTextual())
            return state.dataState(ConstCharArrays.JsonDataState.ofValue(getText(currentNode, key, error), error));
        else
            return state.dataState(getInt(currentNode, key, error));
    }

    private int decodeStateCode(JsonNode currentNode, String key, State state, JsonConverterError error) {
        checkStringOrInt(currentNode, key, error);
        if (error.isFailed())
            return CodecReturnCodes.FAILURE;

        if (currentNode.isTextual())
            return state.code(ConstCharArrays.JsonStateCode.ofValue(getText(currentNode, key, error), error));
        else
            return state.code(getInt(currentNode, key, error));
    }

    @Override
    void encodeRWF(JsonNode dataNode, String key, EncodeIterator iter, JsonConverterError error) {
        if (dataNode.isNull())
            return;

        Buffer encodeTextBuffer = null;
        State encStateValue = JsonFactory.createState();

        try {
            encStateValue.clear();
            if (!dataNode.path(JSON_CODE).isMissingNode())
                decodeStateCode(dataNode.path(JSON_CODE), JSON_CODE, encStateValue, error);

            if (!dataNode.path(JSON_DATA).isMissingNode())
                decodeDataState(dataNode.path(JSON_DATA), JSON_DATA, encStateValue, error);

            if (!dataNode.path(JSON_STREAM).isMissingNode())
                decodeStreamState(dataNode.path(JSON_STREAM), JSON_STREAM, encStateValue, error);

            if (!dataNode.path(JSON_TEXT).isMissingNode()) {
                encodeTextBuffer = JsonFactory.createBuffer();
                encodeTextBuffer.data(dataNode.path(JSON_TEXT).textValue());
                encStateValue.text(encodeTextBuffer);
            }

            int result = encStateValue.encode(iter);
            if (result != SUCCESS) {
                error.setEncodeError(result, key);
                return;
            }
        } finally {
            if (encodeTextBuffer != null)
                JsonFactory.releaseBuffer(encodeTextBuffer);
            JsonFactory.releaseState(encStateValue);
        }
    }

}
