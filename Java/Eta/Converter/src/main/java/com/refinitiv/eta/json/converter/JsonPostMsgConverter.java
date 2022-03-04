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

class JsonPostMsgConverter extends AbstractRsslMessageTypeConverter {
    JsonPostMsgConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    @Override
    boolean encodeJson(DecodeIterator decIter, Object msg, JsonBuffer outBuffer, JsonConverterError error) {

        PostMsg postMsg = (PostMsg) msg;

        boolean res = BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_POSTUSERINFO, outBuffer, true, error);
        if (!converter.processPostUserInfo(postMsg.postUserInfo(), outBuffer, error))
            return false;

        if (postMsg.flags() != PostMsgFlags.NONE) {
            if (postMsg.checkHasPostId()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_POSTID, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(postMsg.postId(), outBuffer, error);
            }

            if (postMsg.checkHasMsgKey()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEY, outBuffer, true, error);
                if (!converter.processMsgKey(decIter, postMsg.msgKey(), outBuffer, postMsg.domainType(), true, error))
                    return false;
            }

            if (postMsg.checkHasSeqNum()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_SEQNUM, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(postMsg.seqNum(), outBuffer, error);
            }

            if (!postMsg.checkPostComplete()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_COMPLETE, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.falseString, outBuffer, false, error);;
            }

            if (postMsg.checkAck()) {
                BufferHelper.writeArrayAndColon(JSON_ACK, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }

            if (postMsg.checkHasPermData()) {
                if (Objects.nonNull(postMsg.permData()) && Objects.nonNull(postMsg.permData().data())) {
                    BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PERMDATA, outBuffer, true, error);
                    if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(postMsg.permData(), outBuffer, error))
                        return false;
                } else {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Empty permission data found");
                    return false;
                }
            }

            if (postMsg.checkHasPartNum()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PARTNUMBER, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(postMsg.partNum(), outBuffer, error);
            }

            if (postMsg.checkHasPostUserRights()) {
                BufferHelper.writeArrayAndColon(JSON_POSTUSERRIGHTS, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(postMsg.postUserRights(), outBuffer, error);
            }

            if (postMsg.checkHasExtendedHdr()) {
                if (Objects.nonNull(postMsg.extendedHeader()) && Objects.nonNull(postMsg.extendedHeader().data())) {
                    BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_EXTHDR, outBuffer, true, error);
                    if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(postMsg.extendedHeader(), outBuffer, error))
                        return false;
                } else {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Empty extended header found");
                    return false;
                }
            }
        }

        if (postMsg.containerType() != DataTypes.NO_DATA) {
            BufferHelper.comma(outBuffer, error);
            if (!converter.getContainerHandler(postMsg.containerType()).encodeJson(decIter, outBuffer, true, null, error))
                return false;
        }

        return res;
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        ((Msg)msg).msgClass(MsgClasses.POST);
        PostMsg postMsg = (PostMsg) msg;
        postMsg.domainType(DomainTypes.MARKET_PRICE);
        //use flags to set properties with default True value

        boolean hasId = false;

        Buffer extendedHeaderBuffer = null;
        Buffer permDataBuffer = null;

        boolean isComplete = true;

        try {
            for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
                String key = it.next();
                JsonNode currentNode = node.path(key);
                switch (key) {
                    case JSON_ID:
                        postMsg.streamId(getInt(currentNode, key, error));
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
                            postMsg.domainType(
                                    ConstCharArrays.JsonDomain.ofValue(
                                            getText(currentNode, key, error)
                                            , error)
                            );
                        else
                            postMsg.domainType(getInt(currentNode, key, error));
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
                        postMsg.containerType(converter.getContainerDataType(key, currentNode, error));
                        break;

                    case JSON_KEY:
                        postMsg.applyHasMsgKey();
                        converter.decodeChunk(RsslMsgChunkType.MSG_KEY_CHUNK, currentNode, postMsg, error);
                        break;

                    case JSON_POSTUSERINFO:
                        converter.decodeChunk(RsslMsgChunkType.POST_USER_INFO_CHUNK, currentNode, postMsg.postUserInfo(), error);
                        break;

                    case JSON_POSTID:
                        postMsg.applyHasPostId();
                        postMsg.postId(getLong(currentNode, key, error));
                        break;

                    case JSON_PARTNUMBER:
                        postMsg.applyHasPartNum();
                        postMsg.partNum(getInt(currentNode, key, error));
                        break;

                    case JSON_POSTUSERRIGHTS:
                        postMsg.applyHasPostUserRights();
                        postMsg.postUserRights(getInt(currentNode, key, error));
                        break;

                    case JSON_PERMDATA:
                        postMsg.applyHasPermData();
                        permDataBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), permDataBuffer, error);
                        if (error.isFailed())
                            break;

                        postMsg.permData(permDataBuffer);
                        break;

                    case JSON_SEQNUM:
                        postMsg.applyHasSeqNum();
                        postMsg.seqNum(getLong(currentNode, key, error));
                        break;

                    case JSON_ACK:
                        if (getBoolean(currentNode, key, error))
                            postMsg.applyAck();
                        break;

                    case JSON_EXTHDR:
                        postMsg.applyHasExtendedHdr();
                        extendedHeaderBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), extendedHeaderBuffer, error);
                        if (error.isFailed())
                            break;

                        postMsg.extendedHeader(extendedHeaderBuffer);
                        break;


                    case JSON_REQKEY:
                        //not yet supported by ETAJ, ignore for now
                        break;

                    case JSON_COMPLETE:
                        isComplete = getBoolean(currentNode, key, error);
                        break;


                    default:
                        processUnexpectedKey(key, error);
                }

                if (error.isFailed())
                    return;
            }

            if (!hasId)
                error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, JSON_ID);

            if (isComplete)
                postMsg.applyPostComplete();

        } finally {
            if (extendedHeaderBuffer != null)
                JsonFactory.releaseBuffer(extendedHeaderBuffer);

            if (permDataBuffer != null)
                JsonFactory.releaseBuffer(permDataBuffer);
        }

    }
}
