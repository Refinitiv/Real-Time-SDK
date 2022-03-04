/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.TransportBuffer;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.Map;

import static com.refinitiv.eta.codec.CodecReturnCodes.*;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonConverterBaseImpl extends JsonAbstractConverter {

    private final ThreadLocal<ObjectMapper> mapper;

    private ThreadLocal<JsonConverterState> currentState = ThreadLocal.withInitial(() -> null);;
    private Map<Integer, AbstractRsslMessageTypeConverter> rsslMsgHandlerMap = new HashMap<>();
    private Map<Integer, AbstractPrimitiveTypeConverter> primitiveHandlerMap = new HashMap<>();
    private Map<Integer, AbstractContainerTypeConverter> containerHandlerMap = new HashMap<>();
    private Map<RsslMsgChunkType, AbstractRsslMessageChunkTypeConverter> rsslMsgChunkHandlerMap = new HashMap<>();
    private boolean catchUnexpectedKeys;
    private boolean catchUnexpectedFids;
    private boolean allowEnumDisplayStrings;
    private boolean useDefaultQoS;
    private boolean expandEnumFields;
    private int defaultServiceId;
    private boolean hasDefaultServiceId;
    private DataDictionary dictionary;
    private ThreadLocal<DictionaryEntry> dictionaryEntry = ThreadLocal.withInitial(() -> null);
    private ThreadLocal<JsonBuffer> jsonOutputBuffer = ThreadLocal.withInitial(() -> new JsonBuffer());
    private ThreadLocal<ByteBufferInputStream> inputStream = ThreadLocal.withInitial(() -> new ByteBufferInputStream());
    private Map<EnumTypeTable, EnumTableDefinition> enumTableDefinitionMap = new HashMap<>(256);

    private static final Map<String, Integer> STRING_TO_RWF_MSG_CLASS = new HashMap<>();;
    private static final Map<String, Integer> STRING_TO_JSON_MSG_CLASS = new HashMap<>();
    
    static final String UPDATE_STR = "Update";
    static final String GENERIC_STR = "Generic";
    static final String REFRESH_STR = "Refresh";
    static final String REQUEST_STR = "Request";
    static final String POST_STR = "Post";
    static final String STATUS_STR = "Status";
    static final String CLOSE_STR = "Close";
    static final String ACK_STR = "Ack";

    static
    {
        STRING_TO_RWF_MSG_CLASS.put(REQUEST_STR, MsgClasses.REQUEST);
        STRING_TO_RWF_MSG_CLASS.put(REFRESH_STR, MsgClasses.REFRESH);
        STRING_TO_RWF_MSG_CLASS.put(STATUS_STR, MsgClasses.STATUS);
        STRING_TO_RWF_MSG_CLASS.put(UPDATE_STR, MsgClasses.UPDATE);
        STRING_TO_RWF_MSG_CLASS.put(CLOSE_STR, MsgClasses.CLOSE);
        STRING_TO_RWF_MSG_CLASS.put(ACK_STR, MsgClasses.ACK);
        STRING_TO_RWF_MSG_CLASS.put(GENERIC_STR, MsgClasses.GENERIC);
        STRING_TO_RWF_MSG_CLASS.put(POST_STR, MsgClasses.POST);

        STRING_TO_JSON_MSG_CLASS.put(JsonMsgClasses.PING_STR, JsonMsgClasses.PING);
        STRING_TO_JSON_MSG_CLASS.put(JsonMsgClasses.PONG_STR, JsonMsgClasses.PONG);
        STRING_TO_JSON_MSG_CLASS.put(JsonMsgClasses.ERROR_STR, JsonMsgClasses.ERROR);
    }

    JsonConverterBaseImpl() {
        mapper = ThreadLocal.withInitial(() -> new ObjectMapper());
        initPrimitiveHandlers();
        initContainerHandlers();
        initRsslMsgChunkHandlers();
        initMessageHandlers();
    }

    private void initMessageHandlers() {
        rsslMsgHandlerMap.put(MsgClasses.REQUEST, new JsonRequestMsgConverter(this));
        rsslMsgHandlerMap.put(MsgClasses.STATUS, new JsonStatusMsgConverter(this));
        rsslMsgHandlerMap.put(MsgClasses.UPDATE, new JsonUpdateMsgConverter(this));
        rsslMsgHandlerMap.put(MsgClasses.REFRESH, new JsonRefreshMsgConverter(this));
        rsslMsgHandlerMap.put(MsgClasses.GENERIC, new JsonGenericMsgConverter(this));
        rsslMsgHandlerMap.put(MsgClasses.ACK, new JsonAckMsgConverter(this));
        rsslMsgHandlerMap.put(MsgClasses.CLOSE, new JsonCloseMsgConverter(this));
        rsslMsgHandlerMap.put(MsgClasses.POST, new JsonPostMsgConverter(this));
    }

    private void initRsslMsgChunkHandlers() {
        rsslMsgChunkHandlerMap.put(RsslMsgChunkType.MSG_KEY_CHUNK, new JsonMsgKeyConverter(this));
        rsslMsgChunkHandlerMap.put(RsslMsgChunkType.PRIORITY_CHUNK, new JsonPriorityConverter(this));
        rsslMsgChunkHandlerMap.put(RsslMsgChunkType.POST_USER_INFO_CHUNK, new JsonPostUserInfoConverter(this));
        rsslMsgChunkHandlerMap.put(RsslMsgChunkType.CONFINFO_CHUNK, new JsonConfInfoConverter(this));
    }

    private void initPrimitiveHandlers() {

        primitiveHandlerMap.put(DataTypes.QOS, new JsonQosConverter(this));
        primitiveHandlerMap.put(DataTypes.STATE, new JsonStateConverter(this));
        primitiveHandlerMap.put(DataTypes.ARRAY, new JsonArrayConverter(this));
        primitiveHandlerMap.put(DataTypes.ENUM, new JsonEnumerationConverter(this));
        primitiveHandlerMap.put(DataTypes.BUFFER, new JsonBufferConverter(this));
        primitiveHandlerMap.put(DataTypes.UTF8_STRING, new JsonUtf8Converter(this));
        primitiveHandlerMap.put(DataTypes.RMTES_STRING, new JsonRMTESConverter(this));
        primitiveHandlerMap.put(DataTypes.ASCII_STRING, new JsonAsciiConverter(this));
        AbstractPrimitiveTypeConverter convInt = new JsonIntConverter(this);
        for (int i = 0; i < convInt.dataTypes.length; i++)
            primitiveHandlerMap.put(convInt.dataTypes[i], convInt);
        AbstractPrimitiveTypeConverter convLong = new JsonLongConverter(this);
        for (int i = 0; i < convLong.dataTypes.length; i++)
            primitiveHandlerMap.put(convLong.dataTypes[i], convLong);
        AbstractPrimitiveTypeConverter convDateTime = new JsonDateTimeConverter(this);
        for (int i = 0; i < convDateTime.dataTypes.length; i++)
            primitiveHandlerMap.put(convDateTime.dataTypes[i], convDateTime);
        primitiveHandlerMap.put(DataTypes.DATE, new JsonDateConverter(this));
        primitiveHandlerMap.put(DataTypes.DATE_4, new JsonDateConverter(this));
        AbstractPrimitiveTypeConverter convTime = new JsonTimeConverter(this);
        for (int i = 0; i < convTime.dataTypes.length; i++)
            primitiveHandlerMap.put(convTime.dataTypes[i], convTime);
        primitiveHandlerMap.put(DataTypes.DOUBLE, new JsonDoubleConverter(this));
        primitiveHandlerMap.put(DataTypes.DOUBLE_8, new JsonDoubleConverter(this));
        primitiveHandlerMap.put(DataTypes.FLOAT, new JsonFloatConverter(this));
        primitiveHandlerMap.put(DataTypes.FLOAT_4, new JsonFloatConverter(this));
        primitiveHandlerMap.put(DataTypes.REAL, new JsonRealConverter(this));
        primitiveHandlerMap.put(DataTypes.REAL_4RB, new JsonRealConverter(this));
        primitiveHandlerMap.put(DataTypes.REAL_8RB, new JsonRealConverter(this));
        primitiveHandlerMap.put(DataTypes.UNKNOWN, new JsonUnknownTypeConverter(this));
    }

    private void initContainerHandlers() {
        containerHandlerMap.put(DataTypes.ELEMENT_LIST, new JsonElementListConverter(this));
        containerHandlerMap.put(DataTypes.FIELD_LIST, new JsonFieldListConverter(this));
        containerHandlerMap.put(DataTypes.FILTER_LIST, new JsonFilterListConverter(this));
        containerHandlerMap.put(DataTypes.VECTOR, new JsonVectorConverter(this));
        containerHandlerMap.put(DataTypes.SERIES, new JsonSeriesConverter(this));
        containerHandlerMap.put(DataTypes.MAP, new JsonMapConverter(this));
        containerHandlerMap.put(DataTypes.OPAQUE, new JsonOpaqueConverter(this));
        containerHandlerMap.put(DataTypes.XML, new JsonXmlConverter(this));
        containerHandlerMap.put(DataTypes.ANSI_PAGE, new JsonAnsiPageConverter(this));
        containerHandlerMap.put(DataTypes.JSON, new JsonJsonConverter(this));
        containerHandlerMap.put(DataTypes.MSG, new JsonMsgConverter(this));
    }

    @Override
    public int parseJsonBuffer(Buffer jsonBuffer, ParseJsonOptions options, JsonConverterError error) {
        if (options.getProtocolType() != JsonProtocol.JSON_JPT_JSON2) {
            return error.setError(JsonConverterErrorCodes.JSON_ERROR, "Protocol type not supported: " + options.getProtocolType());
        }
        return parseJsonBuffer(jsonBuffer.data().array(), error);
    }

    @Override
    public int parseJsonBuffer(TransportBuffer inBuffer, ParseJsonOptions options, JsonConverterError error) {
        if (options.getProtocolType() != JsonProtocol.JSON_JPT_JSON2) {
            return error.setError(JsonConverterErrorCodes.JSON_ERROR, "Protocol type not supported: " + options.getProtocolType());
        }

        return parseJsonBuffer(inBuffer, error);
    }

    @Override
    public int decodeJsonMsg(JsonMsg jsonMsg, DecodeJsonMsgOptions options, JsonConverterError error) {

        if (options.getJsonProtocolType() != JsonProtocol.JSON_JPT_JSON2) {
            return error.setError(JsonConverterErrorCodes.JSON_ERROR, "Protocol type not supported: " + options.getJsonProtocolType());
        }

        jsonMsg.rwfMsg().clear();

        int ret = setCurrentMessageRoot(error);
        if (ret != SUCCESS)
            return ret;

        if (getJsonMsgType(jsonMsg, currentState.get().getWorkingNode(), error) != SUCCESS) {
            currentState.get().setFailedNode(currentState.get().getWorkingNode());
            return FAILURE;
        }

        EncodeIterator encIter = JsonFactory.createEncodeIterator();
        try {
            if (jsonMsg.jsonMsgClass() == JsonMsgClasses.RSSL_MESSAGE) {
                prepareJsonMsgToDecode(jsonMsg);
                encIter.clear();
                encIter.setBufferAndRWFVersion(jsonMsg.rwfMsg().encodedMsgBuffer(), Codec.majorVersion(), Codec.minorVersion());
                decodeRsslMessage(jsonMsg.rwfMsg().msgClass(), currentState.get().getWorkingNode(), jsonMsg.rwfMsg(), error, encIter);
                if (error.isFailed()) {
                    currentState.get().setFailedNode(currentState.get().getWorkingNode());
                    return FAILURE;
                }
            }
        } finally {
            JsonFactory.releaseEncodeIterator(encIter);
        }

        return error.isSuccessful() ? SUCCESS : FAILURE;
    }

    private int setCurrentMessageRoot(JsonConverterError error) {
        JsonConverterState state = currentState.get();
        JsonNode root = state.getCurrentRoot();

        if (Objects.isNull(root))
            return END_OF_CONTAINER;

        if (root.isObject()) {
            state.setCurrentRoot(null);
            state.setWorkingNode(root);
            return SUCCESS;
        } else if (root.isArray()) {
            if (state.getArrayCounter() >= root.size())
                return END_OF_CONTAINER;
            JsonNode currentEntry = root.get(state.getArrayCounter());
            if (currentEntry.isObject()) {
                state.setWorkingNode(currentEntry);
                state.setArrayCounter(state.getArrayCounter() + 1);
                return error.isSuccessful() ? SUCCESS : FAILURE;
            } else if (currentEntry.isArray()) {
                if (state.getEntryCounter() >= currentEntry.size()) {
                    state.setEntryCounter(0);
                    state.setArrayCounter(state.getArrayCounter() + 1);
                    return setCurrentMessageRoot(error);
                }
                state.setWorkingNode(currentEntry.get(state.getEntryCounter()));
                state.setEntryCounter(state.getEntryCounter() + 1);
                return SUCCESS;
            } else {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_PARSE_ERROR, "Error parsing JSON message: expected single message or array of messages, found " + currentEntry.getNodeType().toString() + " type", "root");
                currentState.get().setFailedNode(currentEntry);
                return FAILURE;
            }
        } else {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_PARSE_ERROR, "Error parsing JSON message: expected single message or array of messages, found " + root.getNodeType().toString() + " type", "root");
            currentState.get().setFailedNode(root);
            return FAILURE;
        }
    }

    @Override
    public int convertRWFToJson(Msg inMsg, RWFToJsonOptions options, ConversionResults outResults, JsonConverterError error) {

        if (options.getJsonProtocolType() != JsonProtocol.JSON_JPT_JSON2) {
            error.setError(FAILURE, "Invalid protocol type.");
            return FAILURE;
        }
        JsonBuffer buffer = jsonOutputBuffer.get();
        buffer.position = 0;
        int requiredLenght = estimateJsonLength(inMsg.encodedDataBody().length());
        if (buffer.data == null || buffer.data.length < requiredLenght) {
            JsonFactory.releaseByteArray(buffer.data);
            buffer.data = JsonFactory.createByteArray(requiredLenght);
        }
        DecodeIterator iter = JsonFactory.createDecodeIterator();
        try {
            iter.clear();
            if (inMsg.encodedDataBody() == null || inMsg.encodedDataBody().data() == null) {
                error.setError(FAILURE, "RWF Msg encodedDataBody() is not initialized.");
                return FAILURE;
            }
            iter.setBufferAndRWFVersion(inMsg.encodedDataBody(), Codec.majorVersion(), Codec.minorVersion());
            if (processMsg(iter, inMsg, buffer, error, true)) {
                if (outResults != null)
                    outResults.setLength(buffer.position + 25);
                return SUCCESS;
            } else
                return FAILURE;
        } finally {
            JsonFactory.releaseDecodeIterator(iter);
        }
    }

    @Override
    public int convertRWFToJson(Msg inMsg, RWFToJsonOptions options, JsonConverterError error) {
        return convertRWFToJson(inMsg, options, null, error);
    }

    @Override
    public int getJsonBuffer(Buffer buffer, GetJsonMsgOptions options, JsonConverterError error) {

        JsonBuffer json = jsonOutputBuffer.get();
        if (options.getStreamId() == null || options.isCloseMsg()) {
            if (json.position >= buffer.length())
                buffer.data(ByteBuffer.wrap(new byte[json.position]));
            buffer.data().put(json.data, 0, json.position);
        } else {
            int fullLength = json.position - BufferHelper.getCurrentStreamIdLength(json, error) + BasicPrimitiveConverter.getLongLengthCompare(options.getStreamId());
            byte[] outputData = new byte[fullLength];
            BufferHelper.composeMessage(outputData, options.getStreamId(), json, error);
            buffer.data(ByteBuffer.wrap(outputData));
        }
        return SUCCESS;
    }

    @Override
    public int getJsonBuffer(TransportBuffer buffer, GetJsonMsgOptions options, JsonConverterError error) {
        JsonBuffer json = jsonOutputBuffer.get();
        if (options.getStreamId() == null || options.isCloseMsg()) {
            if (buffer.data().limit() - buffer.data().position() < json.position) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR, "Buffer length is not enough to encode the message, expected " + json.position + " bytes, found " + (buffer.data().limit() - buffer.data().position()) + " bytes");
                return FAILURE;
            }
            buffer.data().put(json.data, 0, json.position);
        } else {
            int fullLength = json.position - BufferHelper.getCurrentStreamIdLength(json, error) + BasicPrimitiveConverter.getLongLengthCompare(options.getStreamId());
            if (buffer.data().limit() - buffer.data().position() < fullLength) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR, "Buffer length is not enough to encode the message, expected " + fullLength + " bytes, found " + (buffer.data().limit() - buffer.data().position()) + " bytes");
                return FAILURE;
            }
            BufferHelper.composeMessage(buffer.data(), options.getStreamId(), json, error);
        }
        return SUCCESS;
    }

    @Override
    public int getErrorMessage(Buffer outBuffer, GetJsonErrorParams params, JsonConverterError error) {

        try {
            byte[] currentMessage = null;
            if (currentState.get().getFailedNode() != null) {
                currentMessage = currentState.get().getFailedNode().toString().getBytes("UTF-8");
            } else if (currentState.get().getFailedMessage() != null) {
                currentMessage = currentState.get().getFailedMessage();
            }

            int countCharsToEscape = BasicPrimitiveConverter.charsToEscapeCount(currentMessage);

            int estimatedMaxLength = (currentMessage != null ? currentMessage.length : 0)
                    + params.getFile().getBytes(StandardCharsets.UTF_8).length
                    + params.getText().getBytes(StandardCharsets.UTF_8).length
                    + BasicPrimitiveConverter.getLongLengthCompare(params.getLine())
                    + countCharsToEscape + 207;

            JsonBuffer buffer;
            try {
                buffer = new JsonBuffer(estimatedMaxLength);
            } catch (Exception e) {

                error.setError(JsonConverterErrorCodes.JSON_ERROR, "Failed to create JSON Error message: " + e.getMessage());
                return FAILURE;
            }

            boolean res = BufferHelper.beginObject(buffer, error)
                    && BufferHelper.writeArrayAndColon(JSON_ID, buffer, false, error)
                    && BasicPrimitiveConverter.writeLong(params.getStreamId(), buffer, error)
                    && BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_TYPE, buffer, true, error)
                    && BufferHelper.writeArray(ConstCharArrays.JSON_ERROR, buffer, true, error)
                    && BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_TEXT, buffer, true, error)
                    && BasicPrimitiveConverter.writeSafeString(params.getText().getBytes(StandardCharsets.UTF_8), buffer, error)
                    && BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_DEBUG, buffer, true, error)
                    && BufferHelper.beginObject(buffer, error);

            if (res && params.getFile() != null) {
                res = BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_FILE, buffer, false, error)
                        && BufferHelper.writeArray(params.getFile(), buffer, true, error);
            }

            if (res && params.getLine() != GetJsonErrorParams.EMPTY_LINE_VALUE) {
                res = BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_LINE, buffer, true, error)
                        && BasicPrimitiveConverter.writeLong(params.getLine(), buffer, error);
            }

            if (res && currentMessage != null) {
                res = BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_MESSAGE, buffer, true, error)
                        && BasicPrimitiveConverter.writeSafeString(currentMessage, buffer, error);
            }

            res = res && BufferHelper.endObject(buffer, error) && BufferHelper.endObject(buffer, error);

            if (res) {
                ByteBuffer output = ByteBuffer.wrap(buffer.data, 0, buffer.position);
                outBuffer.data(output);
            }

            return res ? SUCCESS : FAILURE;
        } catch (UnsupportedEncodingException e) {

            error.setError(JsonConverterErrorCodes.JSON_ERROR, "Failed to create JSON Error message: " + e.getMessage());
            return FAILURE;
        }
    }

    boolean processMsg(DecodeIterator decIter, Msg inMsg, JsonBuffer outBuffer, JsonConverterError error, boolean first) {

        String msgClass = getMsgClassString(inMsg.msgClass());
        if (msgClass == null) {
            error.setError(FAILURE, "Unsupported msg class " + inMsg.msgClass());
            return false;
        }
        if (inMsg.msgClass() != MsgClasses.CLOSE) {
            BufferHelper.beginObject(outBuffer, error);
            BufferHelper.writeArrayAndColon(JSON_ID, outBuffer, false, error);
            BasicPrimitiveConverter.writeLong(inMsg.streamId(), outBuffer, error);
            BufferHelper.writeArrayAndColon(JSON_TYPE, outBuffer, true, error);
            BufferHelper.writeArray(msgClass, outBuffer, true, error);
            if (inMsg.domainType() != DomainTypes.MARKET_PRICE) {
                BufferHelper.writeArrayAndColon(JSON_DOMAIN, outBuffer, true, error);
                String domain = getDomainString(inMsg.domainType());
                if (domain != null)
                    BufferHelper.writeArray(domain, outBuffer, true, error);
                else
                    BasicPrimitiveConverter.writeLong(inMsg.domainType(), outBuffer, error);
            }
        } else { //close message handles everything on its own
            BufferHelper.beginObject(outBuffer, error);
        }

        return error.isSuccessful() && this.getRsslMessageHandler(inMsg.msgClass()).encodeJson(decIter, inMsg, outBuffer, error) && BufferHelper.endObject(outBuffer, error);
    }

    AbstractRsslMessageTypeConverter getRsslMessageHandler(int msgClassTypeId, JsonConverterError error) {
        AbstractRsslMessageTypeConverter handler = getRsslMessageHandler(msgClassTypeId);
        if (handler != null)
            return handler;

        currentState.get().setFailedNode(currentState.get().getWorkingNode());
        error.setError(JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_MESSAGE, "dataType [" + msgClassTypeId + "]" + " no rsslMessageHandler found");
        return null;
    }

    @Override
    AbstractTypeConverter getHandler(int dataType, JsonConverterError error) {
        AbstractTypeConverter handler = getContainerHandler(dataType);
        if (handler != null)
            return handler;

        handler = getPrimitiveHandler(dataType);

        if (handler != null)
            return handler;

        currentState.get().setFailedNode(currentState.get().getWorkingNode());
        error.setError(JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_MESSAGE, "dataType [" + dataType + "]" + " no primitive/container handler found");
        return null;
    }

    @Override
    AbstractTypeConverter getHandler(RsslMsgChunkType rsslMsgChunkType, JsonConverterError error) {
        AbstractTypeConverter handler = getMsgChunkHandler(rsslMsgChunkType);
        if (handler != null)
            return handler;

        currentState.get().setFailedNode(currentState.get().getWorkingNode());
        error.setError(JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_MESSAGE, "dataType [" + rsslMsgChunkType + "]" + " no rssl msgChunk handler found");
        return handler;
    }

    @Override
    AbstractPrimitiveTypeConverter getPrimitiveHandler(int dataType) {

        if (dataType >= DataTypes.UNKNOWN && dataType <= DataTypes.RMTES_STRING
            || dataType >= DataTypes.INT_1 && dataType <= DataTypes.TIME_8) {
            return primitiveHandlerMap.get(dataType);
        }
        return null;
    }

    @Override
    AbstractContainerTypeConverter getContainerHandler(int dataType) {
        return containerHandlerMap.get(dataType);
    }

    @Override
    AbstractRsslMessageChunkTypeConverter getMsgChunkHandler(RsslMsgChunkType rsslMsgChunkType) {
        return rsslMsgChunkHandlerMap.get(rsslMsgChunkType);
    }

    @Override
    AbstractRsslMessageTypeConverter getRsslMessageHandler(int msgClassId) {
        return rsslMsgHandlerMap.get(msgClassId);
    }

    @Override
    int getContainerDataType(String jsonTagName, JsonNode jsonNode, JsonConverterError error) {
        int dataType = getContainerType(jsonTagName);
        if (dataType == DataTypes.NO_DATA) {
            currentState.get().setFailedNode(currentState.get().getWorkingNode());
            error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_CONTAINER_TYPE, "jsonTag " + jsonTagName + " container is not supported");
        }
        if (!checkContainerValueType(dataType, jsonNode)) {
            currentState.get().setFailedNode(currentState.get().getWorkingNode());
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "found value of type " + jsonNode.getNodeType().toString() + " for container type " + jsonTagName);
        }

        return dataType;
    }

    @Override
    int getContainerType(String type) {
        switch (type) {
            case JSON_FIELDLIST:
            case JSON_FIELDS:
                return DataTypes.FIELD_LIST;
            case JSON_FILTERLIST:
                return DataTypes.FILTER_LIST;
            case JSON_ELEMENTS:
            case JSON_ELEMENTLIST:
                return DataTypes.ELEMENT_LIST;
            case JSON_JSON:
                return DataTypes.JSON;
            case JSON_MAP:
                return DataTypes.MAP;
            case JSON_MESSAGE:
                return DataTypes.MSG;
            case JSON_OPAQUE:
                return DataTypes.OPAQUE;
            case JSON_VECTOR:
                return DataTypes.VECTOR;
            case JSON_SERIES:
                return DataTypes.SERIES;
            case JSON_XML:
                return DataTypes.XML;
            case JSON_ANSI:
                return DataTypes.ANSI_PAGE;
            default:
                return DataTypes.NO_DATA;
        }
    }

    private boolean checkContainerValueType(int dataType, JsonNode jsonNode) {
        switch (dataType) {
            case DataTypes.XML:
            case DataTypes.OPAQUE:
                return jsonNode.isTextual() || jsonNode.isNull();
            case DataTypes.JSON:
            case DataTypes.MAP:
            case DataTypes.SERIES:
            case DataTypes.VECTOR:
            case DataTypes.MSG:
            case DataTypes.ELEMENT_LIST:
            case DataTypes.FIELD_LIST:
            case DataTypes.FILTER_LIST:
                return jsonNode.isObject() ||  jsonNode.isNull();
            default:
                return false;
        }
    }

    private int estimateJsonLength(int rwfLength) {

        return rwfLength * 6 + 300;
    }

    private void prepareJsonMsgToDecode(JsonMsg jsonMsg) {
        final Buffer currentBuffer = currentState.get().getCurrentBufferData();
        if (jsonMsg.jsonMsgData().length() <= 0) {
            jsonMsg.jsonMsgData().data(ByteBuffer.allocate(currentBuffer.length()));
        }
        if (jsonMsg.rwfMsg().encodedMsgBuffer().length() <= 0) {
            jsonMsg.rwfMsg().encodedMsgBuffer().data(ByteBuffer.allocate(currentBuffer.length()));
        }
        currentBuffer.copy(jsonMsg.jsonMsgData());
    }

    private String getMsgClassString(int msgClass) {
        switch (msgClass) {
            case MsgClasses.ACK:
                return MC_ACK;
            case MsgClasses.CLOSE:
                return MC_CLOSE;
            case MsgClasses.GENERIC:
                return MC_GENERIC;
            case MsgClasses.POST:
                return MC_POST;
            case MsgClasses.REQUEST:
                return MC_REQUEST;
            case MsgClasses.REFRESH:
                return MC_REFRESH;
            case MsgClasses.STATUS:
                return MC_STATUS;
            case MsgClasses.UPDATE:
                return MC_UPDATE;
            default:
                return null;
        }
    }

    private String getDomainString(int domainType) {
        switch (domainType) {
            case DomainTypes.LOGIN:
                return DOMAIN_STR_LOGIN;
            case DomainTypes.SOURCE:
                return DOMAIN_STR_SOURCE;
            case DomainTypes.DICTIONARY:
                return DOMAIN_STR_DICTIONARY;
            case DomainTypes.MARKET_PRICE:
                return DOMAIN_STR_MARKET_PRICE;
            case DomainTypes.MARKET_BY_ORDER:
                return DOMAIN_STR_MARKET_BY_ORDER;
            case DomainTypes.MARKET_BY_PRICE:
                return DOMAIN_STR_MARKET_BY_PRICE;
            case DomainTypes.MARKET_MAKER:
                return DOMAIN_STR_MARKET_MAKER;
            case DomainTypes.SYMBOL_LIST:
                return DOMAIN_STR_SYMBOL_LIST;
            case DomainTypes.SERVICE_PROVIDER_STATUS:
                return DOMAIN_STR_SERVICE_PROVIDER_STATUS;
            case DomainTypes.HISTORY:
                return DOMAIN_STR_HISTORY;
            case DomainTypes.HEADLINE:
                return DOMAIN_STR_HEADLINE;
            case DomainTypes.STORY:
                return DOMAIN_STR_STORY;
            case DomainTypes.REPLAYHEADLINE:
                return DOMAIN_STR_REPLAYHEADLINE;
            case DomainTypes.REPLAYSTORY:
                return DOMAIN_STR_REPLAYSTORY;
            case DomainTypes.TRANSACTION:
                return DOMAIN_STR_TRANSACTION;
            case DomainTypes.YIELD_CURVE:
                return DOMAIN_STR_YIELD_CURVE;
            case DomainTypes.CONTRIBUTION:
                return DOMAIN_STR_CONTRIBUTION;
            case DomainTypes.PROVIDER_ADMIN:
                return DOMAIN_STR_PROVIDER_ADMIN;
            case DomainTypes.ANALYTICS:
                return DOMAIN_STR_ANALYTICS;
            case DomainTypes.REFERENCE:
                return DOMAIN_STR_REFERENCE;
            case DomainTypes.NEWS_TEXT_ANALYTICS:
                return DOMAIN_STR_NEWS_TEXT_ANALYTICS;
            case DomainTypes.ECONOMIC_INDICATOR:
                return DOMAIN_STR_ECONOMIC_INDICATOR;
            case DomainTypes.POLL:
                return DOMAIN_STR_POLL;
            case DomainTypes.FORECAST:
                return DOMAIN_STR_FORECAST;
            case DomainTypes.MARKET_BY_TIME:
                return DOMAIN_STR_MARKET_BY_TIME;
            case DomainTypes.SYSTEM:
                return DOMAIN_STR_SYSTEM;
            default:
                return null;

        }
    }

    @Override
    synchronized EnumTableDefinition getEnumTableDefinition(EnumTypeTable enumTypeTable)
    {
    	EnumTableDefinition enumTableDefinition = enumTableDefinitionMap.get(enumTypeTable);
    	if(Objects.isNull(enumTableDefinition))
		{
    		enumTableDefinition = new EnumTableDefinition(enumTypeTable.maxValue());
    		enumTableDefinitionMap.put(enumTypeTable, enumTableDefinition);
		}

    	return enumTableDefinition;
    }

    @Override
    ObjectMapper getMapper() {
        return mapper.get();
    }

    @Override
    DictionaryEntry dictionaryEntry() {
        return dictionaryEntry.get();
    }

    @Override
    void dictionaryEntry(DictionaryEntry entry) {
        dictionaryEntry.set(entry);
    }

    @Override
    ServiceNameIdConverter getServiceNameIdConverter() {
        return serviceNameIdConverter;
    }

    @Override
    DataDictionary getDictionary() {
        return dictionary;
    }
    void setDictionary(DataDictionary dictionary) {
        this.dictionary = dictionary;
    }

    @Override
    boolean catchUnexpectedKeys() {
        return catchUnexpectedKeys;
    }
    void catchUnexpectedKeys(boolean enabled) {
        catchUnexpectedKeys = enabled;
    }

    @Override
    boolean catchUnexpectedFids() {
        return catchUnexpectedFids;
    }
    void catchUnexpectedFids(boolean enabled) {
        catchUnexpectedFids = enabled;
    }

    @Override
    boolean allowEnumDisplayStrings() {
        return allowEnumDisplayStrings;
    }
    void allowEnumDisplayStrings(boolean enabled) {
        allowEnumDisplayStrings = enabled;
    }

    @Override
    boolean useDefaultDynamicQoS() {
        return useDefaultQoS;
    }
    void useDefaultDynamicQoS(boolean enabled) {
        useDefaultQoS = enabled;
    }

    @Override
    boolean expandEnumFields() {
        return expandEnumFields;
    }
    void expandEnumFields(boolean enabled) {
        expandEnumFields = enabled;
    }

    @Override
    int getDefaultServiceId() {
        return defaultServiceId;
    }

    @Override
    boolean hasDefaultServiceId() {
        return hasDefaultServiceId;
    }

    @Override
    void setHasDefaultServiceId(boolean value) {
        hasDefaultServiceId = value;
    }

    void setDefaultServiceId(int id) {
        defaultServiceId = id;
    }

    private int parseJsonBuffer(byte[] data, JsonConverterError error) {
        try {
            final JsonConverterState jsonConverterState = Optional
                    .ofNullable(currentState.get())
                    .orElseGet(JsonConverterState::new);
            jsonConverterState.clear();
            currentState.set(jsonConverterState);
            jsonConverterState.setCurrentRoot(mapper.get().readTree(data));
            jsonConverterState.getCurrentBufferData().data(ByteBuffer.wrap(data));
        } catch (IOException e) {
            currentState.get().setFailedMessage(data);
            return error.setError(JsonConverterErrorCodes.JSON_ERROR_PARSE_ERROR, e.getMessage());
        }
        return SUCCESS;
    }

    private int parseJsonBuffer(TransportBuffer buffer, JsonConverterError error) {
        try {
            final JsonConverterState jsonConverterState = Optional
                    .ofNullable(currentState.get())
                    .orElseGet(JsonConverterState::new);
            jsonConverterState.clear();
            currentState.set(jsonConverterState);
            ByteBuffer data = buffer.data();
            ByteBufferInputStream stream = inputStream.get();
            stream.setByteBuffer(data, buffer.dataStartPosition(), data.limit());
            jsonConverterState.setCurrentRoot(mapper.get().readTree(stream));
            jsonConverterState.getCurrentBufferData().data(data);
        } catch (IOException e) {
            byte[] data = new byte[buffer.length()];
            ByteBuffer inData = buffer.data();
            for (int i = 0; i < buffer.length(); i++)
                data[i] = inData.get(i + buffer.dataStartPosition());
            currentState.get().setFailedMessage(data);
            return error.setError(JsonConverterErrorCodes.JSON_ERROR_PARSE_ERROR, e.getMessage());
        }
        return SUCCESS;
    }

    @Override
    public void decodeRsslMessage(int msgClassTypeId, JsonNode path, Object msg, JsonConverterError error, EncodeIterator encodeIterator) {
        AbstractRsslMessageTypeConverter subParser = getRsslMessageHandler(msgClassTypeId, error);
        if (error.isFailed())
            return;

        subParser.decodeJson(path, msg, error);
        if (error.isFailed()) {
            return;
        }
        subParser.encodeRWF(path, "root", (Msg) msg, encodeIterator, error);
    }

    @Override
    public void decodeChunk(int dataType, JsonNode path, Object msg, JsonConverterError error) {
        AbstractTypeConverter subParser = getHandler(dataType, error);
        if (error.isFailed())
            return;

        subParser.decodeJson(path, msg, error);
    }

    protected void decodeChunk(RsslMsgChunkType rsslMsgChunkType, JsonNode path, Object msg, JsonConverterError error) {
        AbstractTypeConverter subParser = getHandler(rsslMsgChunkType, error);
        if (error.isFailed())
            return;

        subParser.decodeJson(path, msg, error);
    }

    @Override
    public void decodeChunk(int dataType, JsonNode dataNode, String key, EncodeIterator iterator, JsonConverterError error) {
        AbstractTypeConverter subParser = getHandler(dataType, error);
        if (subParser == null) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_MESSAGE,"Unknown datatype to parse: [" + dataType + "]");
            return;
        }
        subParser.encodeRWF(dataNode, key, iterator, error);
    }

    @Override
    public int getRwfMsgTypeFromJson(JsonNode type, JsonConverterError error) {
        if (type.isInt()) {
            return type.intValue();
        }
        return stringToMsgClass(type.textValue(), error);
    }

    private int stringToMsgClass(String type, JsonConverterError error) {
        if (!STRING_TO_RWF_MSG_CLASS.containsKey(type)) {
            currentState.get().setFailedNode(currentState.get().getWorkingNode());
            return error.setError(JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_MESSAGE, type, "root");
        }
        return STRING_TO_RWF_MSG_CLASS.get(type);
    }

    @Override
    public int getJsonMsgType(JsonMsg jsonMsg, JsonNode rootNode, JsonConverterError error) {
        int jsonTypeMsg = 0;
        int rwfTypeMsg = 0;
        JsonNode typeNode = null;
        if (!rootNode.isMissingNode()) {
            typeNode = rootNode.path(JSON_TYPE);
            if (!typeNode.isMissingNode()) {
                if (typeNode.isInt()) {
                    jsonTypeMsg = JsonMsgClasses.RSSL_MESSAGE;
                    rwfTypeMsg = typeNode.intValue();
                } else {
                    if (STRING_TO_RWF_MSG_CLASS.containsKey(typeNode.textValue())) {
                        jsonTypeMsg = JsonMsgClasses.RSSL_MESSAGE;
                        rwfTypeMsg = STRING_TO_RWF_MSG_CLASS.get(typeNode.textValue());
                    } else if (STRING_TO_JSON_MSG_CLASS.containsKey(typeNode.textValue())) {
                        jsonTypeMsg = STRING_TO_JSON_MSG_CLASS.get(typeNode.textValue());
                    }
                }
            } else {
                jsonTypeMsg = JsonMsgClasses.RSSL_MESSAGE;
                rwfTypeMsg = MsgClasses.REQUEST;
            }
        }
        if (jsonTypeMsg == 0 || (jsonTypeMsg == JsonMsgClasses.RSSL_MESSAGE && rwfTypeMsg == 0)) {
            currentState.get().setFailedNode(rootNode);
            return error.setError(JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_MSG_TYPE, "Message type is not supported: " + typeNode);
        }
        jsonMsg.jsonMsgClass(jsonTypeMsg);
        if (jsonTypeMsg == JsonMsgClasses.RSSL_MESSAGE) {
            jsonMsg.rwfMsg().msgClass(rwfTypeMsg);
        }
        return SUCCESS;
    }

    @Override
    int getDataType(JsonNode dataType) {

        if (dataType.isInt()) {
            int type = dataType.asInt() + DataTypes.CONTAINER_TYPE_MIN;
            if (type >= DataTypes.UNKNOWN && type <= DataTypes.CONTAINER_TYPE_MAX)
                return type;
            else return FAILURE;
        } else if (dataType.isTextual()) {
            switch (dataType.asText()) {
                case DATA_TYPE_STR_UNKNOWN:
                    return DataTypes.UNKNOWN;
                case DATA_TYPE_STR_INT:
                    return DataTypes.INT;
                case DATA_TYPE_STR_UINT:
                    return DataTypes.UINT;
                case DATA_TYPE_STR_FLOAT:
                    return DataTypes.FLOAT;
                case DATA_TYPE_STR_DOUBLE:
                    return DataTypes.DOUBLE;
                case DATA_TYPE_STR_REAL:
                    return DataTypes.REAL;
                case DATA_TYPE_STR_DATE:
                    return DataTypes.DATE;
                case DATA_TYPE_STR_TIME:
                    return DataTypes.TIME;
                case DATA_TYPE_STR_DATE_TIME:
                    return DataTypes.DATETIME;
                case DATA_TYPE_STR_QOS:
                    return DataTypes.QOS;
                case DATA_TYPE_STR_STATE:
                    return DataTypes.STATE;
                case DATA_TYPE_STR_ENUM:
                    return DataTypes.ENUM;
                case DATA_TYPE_STR_ARRAY:
                    return DataTypes.ARRAY;
                case DATA_TYPE_STR_BUFFER:
                    return DataTypes.BUFFER;
                case DATA_TYPE_STR_ASCII_STRING:
                    return DataTypes.ASCII_STRING;
                case DATA_TYPE_STR_UTF_8_STRING:
                    return DataTypes.UTF8_STRING;
                case DATA_TYPE_STR_RMTES_STRING:
                    return DataTypes.RMTES_STRING;
                case DATA_TYPE_STR_INT_1:
                    return DataTypes.INT_1;
                case DATA_TYPE_STR_UINT_1:
                    return DataTypes.UINT_1;
                case DATA_TYPE_STR_INT_2:
                    return DataTypes.INT_2;
                case DATA_TYPE_STR_UINT_2:
                    return DataTypes.UINT_2;
                case DATA_TYPE_STR_INT_4:
                    return DataTypes.INT_4;
                case DATA_TYPE_STR_UINT_4:
                    return DataTypes.UINT_4;
                case DATA_TYPE_STR_INT_8:
                    return DataTypes.INT_8;
                case DATA_TYPE_STR_UINT_8:
                    return DataTypes.UINT_8;
                case DATA_TYPE_STR_FLOAT_4:
                    return DataTypes.FLOAT_4;
                case DATA_TYPE_STR_DOUBLE_8:
                    return DataTypes.DOUBLE_8;
                case DATA_TYPE_STR_REAL_4_RB:
                    return DataTypes.REAL_4RB;
                case DATA_TYPE_STR_REAL_8_RB:
                    return DataTypes.REAL_8RB;
                case DATA_TYPE_STR_DATE_4:
                    return DataTypes.DATE_4;
                case DATA_TYPE_STR_TIME_3:
                    return DataTypes.TIME_3;
                case DATA_TYPE_STR_TIME_5:
                    return DataTypes.TIME_5;
                case DATA_TYPE_STR_DATE_TIME_7:
                    return DataTypes.DATETIME_7;
                case DATA_TYPE_STR_DATE_TIME_9:
                    return DataTypes.DATETIME_9;
                case DATA_TYPE_STR_DATE_TIME_11:
                    return DataTypes.DATETIME_11;
                case DATA_TYPE_STR_DATE_TIME_12:
                    return DataTypes.DATETIME_12;
                case DATA_TYPE_STR_TIME_7:
                    return DataTypes.TIME_7;
                case DATA_TYPE_STR_TIME_8:
                    return DataTypes.TIME_8;
                case DATA_TYPE_STR_NO_DATA:
                    return DataTypes.NO_DATA;
                case DATA_TYPE_STR_OPAQUE:
                    return DataTypes.OPAQUE;
                case DATA_TYPE_STR_XML:
                    return DataTypes.XML;
                case DATA_TYPE_STR_FIELD_LIST:
                    return DataTypes.FIELD_LIST;
                case DATA_TYPE_STR_ELEMENT_LIST:
                    return DataTypes.ELEMENT_LIST;
                case DATA_TYPE_STR_ANSI_PAGE:
                    return DataTypes.ANSI_PAGE;
                case DATA_TYPE_STR_FILTER_LIST:
                    return DataTypes.FILTER_LIST;
                case DATA_TYPE_STR_VECTOR:
                    return DataTypes.VECTOR;
                case DATA_TYPE_STR_MAP:
                    return DataTypes.MAP;
                case DATA_TYPE_STR_SERIES:
                    return DataTypes.SERIES;
                case DATA_TYPE_STR_MSG:
                    return DataTypes.MSG;
                case DATA_TYPE_STR_JSON:
                    return DataTypes.JSON;
                default:
                    return FAILURE;
            }
        }

        return FAILURE;
    }

    @Override
    int getPayloadType(JsonNode node) {
        JsonNode curr;
        for (int i = 0; i < node.size(); i++) {
            curr = node.get(i);
            for (Iterator<String> it = curr.fieldNames(); it.hasNext(); ) {
                String name = it.next();
                switch (name) {
                    case ConstCharArrays.JSON_FIELDLIST:
                    case ConstCharArrays.JSON_FIELDS:
                        return DataTypes.FIELD_LIST;
                    case ConstCharArrays.JSON_FILTERLIST:
                        return DataTypes.FILTER_LIST;
                    case ConstCharArrays.JSON_ELEMENTS:
                    case ConstCharArrays.JSON_ELEMENTLIST:
                        return DataTypes.ELEMENT_LIST;
                    case ConstCharArrays.JSON_JSON:
                        return DataTypes.JSON;
                    case ConstCharArrays.JSON_MAP:
                        return DataTypes.MAP;
                    case ConstCharArrays.JSON_MESSAGE:
                        return DataTypes.MSG;
                    case ConstCharArrays.JSON_OPAQUE:
                        return DataTypes.OPAQUE;
                    case ConstCharArrays.JSON_VECTOR:
                        return DataTypes.VECTOR;
                    case ConstCharArrays.JSON_SERIES:
                        return DataTypes.SERIES;
                    case ConstCharArrays.JSON_XML:
                        return DataTypes.XML;
                    case ConstCharArrays.JSON_ANSI:
                        return DataTypes.ANSI_PAGE;
                    default:
                        break;
                }
            }
        }

        return DataTypes.NO_DATA;
    }

    @Override
    public String getDataType(int dataType) {

        if (dataType >= DataTypes.UNKNOWN && dataType <= DataTypes.RMTES_STRING)
            return ConstCharArrays.dataTypeStrings[dataType];
        else if (dataType >= DataTypes.INT_1 && dataType <= DataTypes.TIME_8)
            return ConstCharArrays.dataTypeStrings[dataType - INT_1_SHIFT];
        else if (dataType >= DataTypes.OPAQUE && dataType <= DataTypes.CONTAINER_TYPE_MAX)
            return ConstCharArrays.dataTypeStrings[dataType - OPAQUE_SHIFT];
        else if (dataType == DataTypes.NO_DATA)
            return ConstCharArrays.dataTypeStrings[NO_DATA_POSITION];
        return null;
    }

    @Override
    boolean processMsgKey(DecodeIterator decIter, Object msgKey, JsonBuffer outBuffer, int domain, boolean wantServiceName, JsonConverterError error) {
        return ((JsonMsgKeyConverter)rsslMsgChunkHandlerMap.get(RsslMsgChunkType.MSG_KEY_CHUNK)).encodeJson(decIter, msgKey, outBuffer, domain, wantServiceName, error);
    }

    @Override
    boolean processPostUserInfo(PostUserInfo info, JsonBuffer outBuffer, JsonConverterError error) {
        return ((JsonPostUserInfoConverter) rsslMsgChunkHandlerMap.get(RsslMsgChunkType.POST_USER_INFO_CHUNK)).writeToJson(outBuffer, info, error);
    }
}
