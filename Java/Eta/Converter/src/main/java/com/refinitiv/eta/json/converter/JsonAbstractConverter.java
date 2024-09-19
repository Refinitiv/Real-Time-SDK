/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.TransportBuffer;

public abstract class JsonAbstractConverter implements JsonConverter {

    protected ServiceNameIdConverter serviceNameIdConverter;

    public abstract int parseJsonBuffer(Buffer inBuffer, ParseJsonOptions options, JsonConverterError error);
    public abstract int parseJsonBuffer(TransportBuffer inBuffer, ParseJsonOptions options, JsonConverterError error);
    public abstract int decodeJsonMsg(JsonMsg jsonMsg, DecodeJsonMsgOptions options, JsonConverterError error);

    public abstract int convertRWFToJson(Msg inMsg, RWFToJsonOptions options, ConversionResults outResults, JsonConverterError error);
    public abstract int convertRWFToJson(Msg inMsg, RWFToJsonOptions options, JsonConverterError error);
    public abstract int getJsonBuffer(Buffer result, GetJsonMsgOptions options, JsonConverterError error);
    public abstract int getJsonBuffer(TransportBuffer buffer, GetJsonMsgOptions options, JsonConverterError error);

    public abstract int getErrorMessage(Buffer outBuffer, GetJsonErrorParams params, JsonConverterError error);

    abstract AbstractPrimitiveTypeConverter getPrimitiveHandler(int dataType);
    abstract AbstractContainerTypeConverter getContainerHandler(int dataType);
    abstract AbstractRsslMessageChunkTypeConverter getMsgChunkHandler(RsslMsgChunkType rsslMsgChunkType);
    abstract AbstractRsslMessageTypeConverter getRsslMessageHandler(int msgClassId);
    abstract AbstractTypeConverter getHandler(int dataType, JsonConverterError error);
    abstract AbstractTypeConverter getHandler(RsslMsgChunkType rsslMsgChunkType, JsonConverterError error);
    abstract int getContainerDataType(String jsonTagName, JsonNode jsonNode, JsonConverterError error);
    abstract DataDictionary getDictionary();
    abstract ObjectMapper getMapper();
    abstract DictionaryEntry dictionaryEntry();
    abstract void dictionaryEntry(DictionaryEntry entry);
    abstract ServiceNameIdConverter getServiceNameIdConverter();

    public abstract int getRwfMsgTypeFromJson(JsonNode type, JsonConverterError error);
    public abstract int getJsonMsgType(JsonMsg jsonMsg, JsonNode node, JsonConverterError error);

    abstract boolean catchUnexpectedKeys();
    abstract boolean catchUnexpectedFids();
    abstract boolean allowEnumDisplayStrings();
    abstract boolean useDefaultDynamicQoS();
    abstract boolean expandEnumFields();
    abstract int getDefaultServiceId();
    abstract boolean hasDefaultServiceId();
    abstract void setHasDefaultServiceId(boolean value);
    abstract String getDataType(int dataType);
    abstract int getDataType(JsonNode dataType);
    abstract int getContainerType(String type);
    abstract int getPayloadType(JsonNode node);
    abstract boolean processMsg(DecodeIterator decIter, Msg inMsg, JsonBuffer outBuffer, JsonConverterError error, boolean first);
    abstract boolean processMsgKey(DecodeIterator decIter, Object msgKey, JsonBuffer outBuffer, int domain, boolean wantServiceName, JsonConverterError error);
    abstract boolean processPostUserInfo(PostUserInfo info, JsonBuffer outBuffer, JsonConverterError error);
    abstract EnumTableDefinition getEnumTableDefinition(EnumTypeTable enumTypeTable);

    void setServiceNameIdConverter(ServiceNameIdConverter serviceNameIdConverter) {
        this.serviceNameIdConverter = serviceNameIdConverter;
    }

    abstract protected void decodeRsslMessage(int dataType, JsonNode path, Object msg, JsonConverterError error, EncodeIterator encodeIterator);
    abstract protected void decodeChunk(int dataType, JsonNode path, Object msg, JsonConverterError error);
    abstract protected void decodeChunk(RsslMsgChunkType rsslMsgChunkType, JsonNode path, Object msg, JsonConverterError error);
    abstract protected void decodeChunk(int dataType, JsonNode path, String key, EncodeIterator iterator, JsonConverterError error);

    int serviceNameToId(String serviceName, JsonConverterError error) {
       if (serviceNameIdConverter == null) {
           error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "ServiceNameToId callback was not configured, cannot get id for service '" + serviceName + "'");
           return -1; // return -1 to indicate invalid service ID
       }

       return serviceNameIdConverter.serviceNameToId(serviceName, error);
    }
}
