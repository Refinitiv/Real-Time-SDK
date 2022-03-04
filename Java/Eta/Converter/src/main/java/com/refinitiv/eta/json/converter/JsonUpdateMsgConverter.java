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
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.UpdateEventTypes;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Objects;

import static com.refinitiv.eta.json.converter.BufferConverter.decodeFromBase64;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonUpdateMsgConverter extends AbstractRsslMessageTypeConverter {
    JsonUpdateMsgConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    private static HashMap<Integer, String> updateEventTypeMap = new HashMap<>();

    static {
        updateEventTypeMap.put(UpdateEventTypes.UNSPECIFIED, ConstCharArrays.UPD_EVENT_TYPE_UNSPECIFIED);
        updateEventTypeMap.put(UpdateEventTypes.QUOTE, ConstCharArrays.UPD_EVENT_TYPE_QUOTE);
        updateEventTypeMap.put(UpdateEventTypes.TRADE, ConstCharArrays.UPD_EVENT_TYPE_TRADE);
        updateEventTypeMap.put(UpdateEventTypes.NEWS_ALERT, ConstCharArrays.UPD_EVENT_TYPE_NEWS_ALERT);
        updateEventTypeMap.put(UpdateEventTypes.VOLUME_ALERT, ConstCharArrays.UPD_EVENT_TYPE_VOLUME_ALERT);
        updateEventTypeMap.put(UpdateEventTypes.ORDER_INDICATION, ConstCharArrays.UPD_EVENT_TYPE_ORDER_INDICATION);
        updateEventTypeMap.put(UpdateEventTypes.CLOSING_RUN, ConstCharArrays.UPD_EVENT_TYPE_CLOSING_RUN);
        updateEventTypeMap.put(UpdateEventTypes.CORRECTION, ConstCharArrays.UPD_EVENT_TYPE_CORRECTION);
        updateEventTypeMap.put(UpdateEventTypes.MARKET_DIGEST, ConstCharArrays.UPD_EVENT_TYPE_MARKET_DIGEST);
        updateEventTypeMap.put(UpdateEventTypes.QUOTES_TRADE, ConstCharArrays.UPD_EVENT_TYPE_QUOTES_TRADE);
        updateEventTypeMap.put(UpdateEventTypes.MULTIPLE, ConstCharArrays.UPD_EVENT_TYPE_MULTIPLE);
        updateEventTypeMap.put(UpdateEventTypes.VERIFY, ConstCharArrays.UPD_EVENT_TYPE_VERIFY);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        ((Msg)msg).msgClass(MsgClasses.UPDATE);
        UpdateMsg updateMsg = (UpdateMsg) msg;
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        //use flags to set properties with default True value

        boolean hasId = false;

        Buffer extendedHeaderBuffer = null;
        Buffer permDataBuffer = null;

        try {
            for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
                String key = it.next();
                JsonNode currentNode = node.path(key);
                switch (key) {
                    case JSON_ID:
                        updateMsg.streamId(getInt(currentNode, key, error));
                        hasId = true;
                        break;
                    case JSON_TYPE:
                        //message type already considered on top level
                        break;

                    case JSON_DOMAIN:
                        checkStringOrInt(currentNode, key, error);
                        if (error.isFailed())
                            break;

                        if (currentNode.isTextual())
                            updateMsg.domainType(
                                    ConstCharArrays.JsonDomain.ofValue(
                                            getText(currentNode, key, error)
                                            , error)
                            );
                        else
                            updateMsg.domainType(getInt(currentNode, key, error));
                        break;

                    //treat as body
                    case JSON_ELEMENTS:
                    case JSON_FIELDS:
                    case JSON_FILTERLIST:
                    case JSON_MAP:
                    case JSON_MESSAGE:
                    case JSON_VECTOR:
                    case JSON_JSON:
                    case JSON_XML:
                    case JSON_OPAQUE:
                    case JSON_SERIES:
                        updateMsg.containerType(converter.getContainerDataType(key, currentNode, error));
                        break;

                    case JSON_KEY:
                        updateMsg.applyHasMsgKey();
                        converter.decodeChunk(RsslMsgChunkType.MSG_KEY_CHUNK, currentNode, updateMsg, error);
                        break;
                    case JSON_EXTHDR:
                        updateMsg.applyHasExtendedHdr();
                        extendedHeaderBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), extendedHeaderBuffer, error);
                        if (error.isFailed())
                            break;

                        updateMsg.extendedHeader(extendedHeaderBuffer);
                        break;

                    case JSON_POSTUSERINFO:
                        updateMsg.applyHasPostUserInfo();
                        converter.decodeChunk(RsslMsgChunkType.POST_USER_INFO_CHUNK, currentNode, updateMsg.postUserInfo(), error);
                        break;

                    case JSON_PERMDATA:
                        updateMsg.applyHasPermData();
                        permDataBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), permDataBuffer, error);
                        if (error.isFailed())
                            break;

                        updateMsg.permData(permDataBuffer);
                        break;

                    case JSON_SEQNUM:
                        updateMsg.applyHasSeqNum();
                        updateMsg.seqNum(getLong(currentNode, key, error));
                        break;

                    case JSON_CONFINFO:
                        updateMsg.applyHasConfInfo();
                        converter.decodeChunk(RsslMsgChunkType.CONFINFO_CHUNK, currentNode, updateMsg, error);
                        break;

                    case JSON_DONOTCACHE:
                        if (getBoolean(currentNode, key, error))
                            updateMsg.applyDoNotCache();
                        break;

                    case JSON_DONOTCONFLATE:
                        if (getBoolean(currentNode, key, error))
                            updateMsg.applyDoNotConflate();
                        break;

                    case JSON_DONOTRIPPLE:
                        if (getBoolean(currentNode, key, error))
                            updateMsg.applyDoNotRipple();
                        break;

                    case JSON_UPDATETYPE:
                        checkStringOrInt(currentNode, key, error);
                        if(error.isFailed())
                            break;

                        if (currentNode.isTextual())
                            updateMsg.updateType(ConstCharArrays.JsonUpdateType.ofValue(getText(currentNode, key, error), error));
                        else
                            updateMsg.updateType(getInt(currentNode, key, error));
                        break;

                    default:
                        processUnexpectedKey(key, error);
                }

                if (error.isFailed())
                    return;
            }

                if (!hasId)
                error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, JSON_ID);
        } finally {
            if (extendedHeaderBuffer != null)
                JsonFactory.releaseBuffer(extendedHeaderBuffer);

            if (permDataBuffer != null)
                JsonFactory.releaseBuffer(permDataBuffer);
        }

    }

    @Override
    boolean encodeJson(DecodeIterator decIter, Object msg, JsonBuffer outBuffer, JsonConverterError error) {

        UpdateMsg upd = (UpdateMsg) msg;
        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_UPDATETYPE, outBuffer, true, error);
        String updateType = getUpdateEventType(upd.updateType());
        if (updateType != null)
            BufferHelper.writeArray(updateType, outBuffer, true, error);
        else
            BasicPrimitiveConverter.writeLong(upd.updateType(), outBuffer, error);

        if (upd.checkDoNotConflate()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_DONOTCONFLATE, outBuffer, true, error);
            BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
        }

        if (upd.checkDoNotRipple()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_DONOTRIPPLE, outBuffer, true, error);
            BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
        }

        if (upd.checkDiscardable()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_DISCARDABLE, outBuffer, true, error);
            BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
        }

        if (upd.checkDoNotCache()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_DONOTCACHE, outBuffer, true, error);
            BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
        }

        if (upd.checkHasMsgKey()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEY, outBuffer, true, error);
            if (!converter.processMsgKey(decIter, upd.msgKey(), outBuffer, upd.domainType(), true, error))
                return false;
        }

        if (upd.checkHasPostUserInfo()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_POSTUSERINFO, outBuffer, true, error);
            if (!converter.processPostUserInfo(upd.postUserInfo(), outBuffer, error))
                return false;
        }

        if (upd.checkHasPermData()) {
            if (Objects.nonNull(upd.permData()) && Objects.nonNull(upd.permData().data())) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PERMDATA, outBuffer, true, error);
                if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(upd.permData(), outBuffer, error))
                    return false;
            } else {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Empty permission data found");
                return false;
            }
        }

        if (upd.checkHasSeqNum()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_SEQNUM, outBuffer, true, error);
            BasicPrimitiveConverter.writeLong(upd.seqNum(), outBuffer, error);
        }

        if (upd.checkHasConfInfo()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_CONFINFO, outBuffer, true, error);
            BufferHelper.beginObject(outBuffer, error);
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_COUNT, outBuffer, false, error);
            BasicPrimitiveConverter.writeLong(upd.conflationCount(), outBuffer, error);
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_TIME, outBuffer, true, error);
            BasicPrimitiveConverter.writeLong(upd.conflationTime(), outBuffer, error);
            BufferHelper.endObject(outBuffer, error);
        }

        if (upd.checkHasExtendedHdr()) {
            if (Objects.nonNull(upd.extendedHeader()) && Objects.nonNull(upd.extendedHeader().data())) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_EXTHDR, outBuffer, true, error);
                if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(upd.extendedHeader(), outBuffer, error))
                    return false;
            } else {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Empty extended header found");
                return false;
            }
        }

        if (upd.containerType() != DataTypes.NO_DATA) {
            BufferHelper.comma(outBuffer, error);
            converter.getContainerHandler(upd.containerType()).encodeJson(decIter, outBuffer, true, null, error);
        }

        return error.isSuccessful();
    }

    private String getUpdateEventType(int type) {
        return updateEventTypeMap.get(type);
    }

}
