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
import com.refinitiv.eta.rdm.DomainTypes;

import java.util.Iterator;
import java.util.Objects;

import static com.refinitiv.eta.json.converter.BufferConverter.decodeFromBase64;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonStatusMsgConverter extends AbstractRsslMessageTypeConverter {
    JsonStatusMsgConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        ((Msg)msg).msgClass(MsgClasses.STATUS);
        StatusMsg statusMsg = (StatusMsg) msg;
        statusMsg.domainType(DomainTypes.MARKET_PRICE);
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
                        statusMsg.streamId(getInt(currentNode, key, error));
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
                            statusMsg.domainType(
                                    ConstCharArrays.JsonDomain.ofValue(
                                            getText(currentNode, key, error)
                                            , error)
                            );
                        else
                            statusMsg.domainType(getInt(currentNode, key, error));
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
                        statusMsg.containerType(converter.getContainerDataType(key, currentNode, error));
                        break;

                    case JSON_KEY:
                        statusMsg.applyHasMsgKey();
                        converter.decodeChunk(RsslMsgChunkType.MSG_KEY_CHUNK, currentNode, statusMsg, error);
                        break;

                    case JSON_REQKEY:
                        //not yet supported by ETAJ, ignore for now
                        break;

                    case JSON_EXTHDR:
                        statusMsg.applyHasExtendedHdr();
                        extendedHeaderBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), extendedHeaderBuffer, error);
                        if (error.isFailed())
                            break;

                        statusMsg.extendedHeader(extendedHeaderBuffer);
                        break;

                    case JSON_POSTUSERINFO:
                        statusMsg.applyHasPostUserInfo();
                        converter.decodeChunk(RsslMsgChunkType.POST_USER_INFO_CHUNK, currentNode, statusMsg.postUserInfo(), error);
                        break;

                    case JSON_PERMDATA:
                        statusMsg.applyHasPermData();
                        permDataBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), permDataBuffer, error);
                        if (error.isFailed())
                            break;

                        statusMsg.permData(permDataBuffer);
                        break;

                    case JSON_PRIVATE:
                        if (getBoolean(currentNode, key, error))
                            statusMsg.applyPrivateStream();
                        break;

                    case JSON_STATE:
                        statusMsg.applyHasState();
                        converter.decodeChunk(DataTypes.STATE, currentNode, statusMsg.state(), error);
                        break;

                    case JSON_CLEARCACHE:
                        if (getBoolean(currentNode, key, error))
                            statusMsg.applyClearCache();
                        break;

                    case JSON_QUALIFIED:
                        if (getBoolean(currentNode, key, error))
                            statusMsg.applyQualifiedStream();
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

        StatusMsg statusMsg = (StatusMsg) msg;

        if (statusMsg.flags() != RefreshMsgFlags.NONE) {

            if (statusMsg.checkHasMsgKey()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEY, outBuffer, true, error);
                converter.processMsgKey(decIter, statusMsg.msgKey(), outBuffer, statusMsg.domainType(), true, error);
            }

            if (statusMsg.checkHasState()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_STATE, outBuffer, true, error);
                if (!((JsonStateConverter)converter.getPrimitiveHandler(DataTypes.STATE)).writeToJson(statusMsg.state(), outBuffer, error))
                    return false;
            }

            if (statusMsg.checkClearCache()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_CLEARCACHE, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }

            if (statusMsg.checkPrivateStream()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PRIVATE, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }

            if (statusMsg.checkQualifiedStream()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_QUALIFIED, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }

            if (statusMsg.checkHasPostUserInfo()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_POSTUSERINFO, outBuffer, true, error);
                if (!converter.processPostUserInfo(statusMsg.postUserInfo(), outBuffer, error))
                    return false;
            }

            if (statusMsg.checkHasExtendedHdr()) {
                if (Objects.nonNull(statusMsg.extendedHeader()) && Objects.nonNull(statusMsg.extendedHeader().data())) {
                    BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_EXTHDR, outBuffer, true, error);
                    if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(statusMsg.extendedHeader(), outBuffer, error))
                        return false;
                } else {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Empty extended header found");
                    return false;
                }
            }

            if (statusMsg.checkHasPermData()) {
                if (Objects.nonNull(statusMsg.permData()) && Objects.nonNull(statusMsg.permData().data())) {
                    BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PERMDATA, outBuffer, true, error);
                    if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(statusMsg.permData(), outBuffer, error))
                        return false;
                } else {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Empty permission data found");
                    return false;
                }
            }
        }

        if (statusMsg.containerType() != DataTypes.NO_DATA) {
            BufferHelper.comma(outBuffer, error);
            if (!converter.getContainerHandler(statusMsg.containerType()).encodeJson(decIter, outBuffer, true, null, error))
                return false;
        }

        return error.isSuccessful();
    }

}
