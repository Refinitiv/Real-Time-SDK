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

import java.util.Iterator;
import java.util.Objects;

import static com.refinitiv.eta.json.converter.BufferConverter.decodeFromBase64;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonRefreshMsgConverter extends AbstractRsslMessageTypeConverter {
    JsonRefreshMsgConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    @Override
    boolean encodeJson(DecodeIterator decIter, Object msg, JsonBuffer outBuffer, JsonConverterError error) {

        RefreshMsg refMsg = (RefreshMsg) msg;
        MsgKey key = refMsg.msgKey();

        if (key != null) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEY, outBuffer, true, error);
            if (!converter.processMsgKey(decIter, key, outBuffer, refMsg.domainType(), true, error))
                return false;
        }

        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_STATE, outBuffer, true, error);
        if (!((JsonStateConverter)converter.getPrimitiveHandler(DataTypes.STATE)).writeToJson(refMsg.state(), outBuffer, error))
            return false;

        if (refMsg.flags() != RefreshMsgFlags.NONE) {
            //we don't have HAS_REQ_MSG_KEY flag in RefreshMsgFlags in Java version
            if (refMsg.checkHasExtendedHdr()) {
                if (Objects.nonNull(refMsg.extendedHeader()) && Objects.nonNull(refMsg.extendedHeader().data())) {
                    BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_EXTHDR, outBuffer, true, error);
                    if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(refMsg.extendedHeader(), outBuffer, error))
                        return false;
                } else {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Empty extended header found");
                    return false;
                }
            }

            if (!refMsg.checkRefreshComplete()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_COMPLETE, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.falseString, outBuffer, false, error);
            }

            if (refMsg.checkHasQos()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_QOS, outBuffer, true, error);
                if (!((JsonQosConverter) converter.getPrimitiveHandler(DataTypes.QOS)).writeToJson(refMsg.qos(), outBuffer, error))
                    return false;
            }

            if (!refMsg.checkClearCache()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_CLEARCACHE, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.falseString, outBuffer, false, error);
            }

            if (refMsg.checkDoNotCache()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_DONOTCACHE, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }

            if (refMsg.checkPrivateStream()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PRIVATE, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }

            if (refMsg.checkQualifiedStream()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_QUALIFIED, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }

            if (refMsg.checkHasPostUserInfo()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_POSTUSERINFO, outBuffer, true, error);
                if (!converter.processPostUserInfo(refMsg.postUserInfo(), outBuffer, error))
                    return false;
            }

            if (refMsg.checkHasPartNum()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PARTNUMBER, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(refMsg.partNum(), outBuffer, error);
            }

            if (refMsg.checkHasPermData()) {
                if (Objects.nonNull(refMsg.permData()) && Objects.nonNull(refMsg.permData().data())) {
                    BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PERMDATA, outBuffer, true, error);
                    if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(refMsg.permData(), outBuffer, error))
                        return false;
                } else {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Empty permission data found");
                    return false;
                }
            }

            if (refMsg.checkHasSeqNum()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_SEQNUM, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(refMsg.seqNum(), outBuffer, error);
            }

            if (!refMsg.checkSolicited()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_SOLICITED, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.falseString, outBuffer, false, error);
            }
        }

        if (refMsg.containerType() != DataTypes.NO_DATA) {
            BufferHelper.comma(outBuffer, error);
            if (!converter.getContainerHandler(refMsg.containerType()).encodeJson(decIter, outBuffer, true, null, error))
                return false;
        }

        return true;
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        RefreshMsg refreshMsg = (RefreshMsg) msg;
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        //use flags to set properties with default True value

        boolean hasId = false;

        Buffer extendedHeaderBuffer = null;
        Buffer permDataBuffer = null;

        boolean hasState = false;
        boolean isSolicited = true;
        boolean isComplete = true;
        boolean isClearCache = true;

        try {
            for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
                String key = it.next();
                JsonNode currentNode = node.path(key);
                switch (key) {
                    case JSON_ID:
                        refreshMsg.streamId(getInt(currentNode, key, error));
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
                            refreshMsg.domainType(
                                    ConstCharArrays.JsonDomain.ofValue(
                                            getText(currentNode, key, error)
                                            , error)
                            );
                        else
                            refreshMsg.domainType(getInt(currentNode, key, error));
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
                        refreshMsg.containerType(converter.getContainerDataType(key, currentNode, error));
                        break;

                    case JSON_KEY:
                        refreshMsg.applyHasMsgKey();
                        converter.decodeChunk(RsslMsgChunkType.MSG_KEY_CHUNK, currentNode, refreshMsg, error);
                        break;
                    case JSON_EXTHDR:
                        refreshMsg.applyHasExtendedHdr();
                        extendedHeaderBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), extendedHeaderBuffer, error);
                        if (error.isFailed())
                            break;

                        refreshMsg.extendedHeader(extendedHeaderBuffer);
                        break;

                    case JSON_POSTUSERINFO:
                        refreshMsg.applyHasPostUserInfo();
                        converter.decodeChunk(RsslMsgChunkType.POST_USER_INFO_CHUNK, currentNode, refreshMsg.postUserInfo(), error);
                        break;

                    case JSON_PERMDATA:
                        refreshMsg.applyHasPermData();
                        permDataBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), permDataBuffer, error);
                        if (error.isFailed())
                            break;

                        refreshMsg.permData(permDataBuffer);
                        break;

                    case JSON_PRIVATE:
                        if (getBoolean(currentNode, key, error))
                            refreshMsg.applyPrivateStream();
                        break;

                    case JSON_PARTNUMBER:
                        refreshMsg.applyHasPartNum();
                        refreshMsg.partNum(getInt(currentNode, key, error));
                        break;

                    case JSON_STATE:
                        hasState = true;
                        converter.decodeChunk(DataTypes.STATE, currentNode, refreshMsg.state(), error);
                        break;

                    case JSON_SEQNUM:
                        refreshMsg.applyHasSeqNum();
                        refreshMsg.seqNum(getLong(currentNode, key, error));
                        break;

                    case JSON_SOLICITED:
                        isSolicited = getBoolean(currentNode, key, error);
                        break;

                    case JSON_COMPLETE:
                        isComplete = getBoolean(currentNode, key, error);
                        break;

                    case JSON_CLEARCACHE:
                        isClearCache = getBoolean(currentNode, key, error);
                        break;

                    case JSON_QOS:
                        refreshMsg.applyHasQos();
                        converter.decodeChunk(DataTypes.QOS, currentNode, refreshMsg.qos(), error);
                        break;


                    case JSON_QUALIFIED:
                        if (getBoolean(currentNode, key, error))
                            refreshMsg.applyQualifiedStream();
                        break;

                    case JSON_REQKEY:
                        //not yet supported by ETAJ, ignore for now
                        break;

                    default:
                        processUnexpectedKey(key, error);
                }

                if (error.isFailed())
                    return;
            }

            if (!hasId)
                error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, JSON_ID);

            if (isClearCache)
                refreshMsg.applyClearCache();

            if (!hasState) {
                //default state
                refreshMsg.state().dataState(DataStates.OK);
                refreshMsg.state().streamState(StreamStates.OPEN);
            }

            if (isSolicited)
                refreshMsg.applySolicited();

            if (isComplete)
                refreshMsg.applyRefreshComplete();

            if (isClearCache)
                refreshMsg.applyClearCache();

        } finally {
            if (extendedHeaderBuffer != null)
                JsonFactory.releaseBuffer(extendedHeaderBuffer);

            if (permDataBuffer != null)
                JsonFactory.releaseBuffer(permDataBuffer);
        }

    }

}
