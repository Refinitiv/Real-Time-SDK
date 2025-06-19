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
import com.refinitiv.eta.rdm.DomainTypes;

import java.util.Iterator;
import java.util.Objects;

import static com.refinitiv.eta.json.converter.BufferConverter.decodeFromBase64;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonGenericMsgConverter extends AbstractRsslMessageTypeConverter {
    JsonGenericMsgConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    @Override
    boolean encodeJson(DecodeIterator decIter, Object msg, JsonBuffer outBuffer, JsonConverterError error) {

        GenericMsg genericMsg = (GenericMsg) msg;

        if (genericMsg.flags() != GenericMsgFlags.NONE) {
            if (genericMsg.checkHasMsgKey()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEY, outBuffer, true, error);
                converter.processMsgKey(decIter, genericMsg.msgKey(), outBuffer, genericMsg.domainType(), true, error);
            }

            if (genericMsg.checkHasSeqNum()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_SEQNUM, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(genericMsg.seqNum(), outBuffer, error);
            }

            if (genericMsg.checkHasSecondarySeqNum()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_SECSEQNUM, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(genericMsg.secondarySeqNum(), outBuffer, error);
            }

            if (genericMsg.checkHasPartNum()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PARTNUMBER, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(genericMsg.partNum(), outBuffer, error);
            }

            if (genericMsg.checkHasPermData()) {
                if (Objects.nonNull(genericMsg.permData()) && Objects.nonNull(genericMsg.permData().data())) {
                    BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PERMDATA, outBuffer, true, error);
                    if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(genericMsg.permData(), outBuffer, error))
                        return false;
                } else {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Empty permission data found");
                    return false;
                }
            }

            if (genericMsg.checkHasExtendedHdr()) {
                if (Objects.nonNull(genericMsg.extendedHeader()) && Objects.nonNull(genericMsg.extendedHeader().data())) {
                    BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_EXTHDR, outBuffer, true, error);
                    if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(genericMsg.extendedHeader(), outBuffer, error))
                        return false;
                } else {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Empty extended header found");
                    return false;
                }
            }
        }

        if (!genericMsg.checkMessageComplete()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_COMPLETE, outBuffer, true, error);
            BufferHelper.writeArray(ConstCharArrays.falseString, outBuffer, false, error);
        }

        if (genericMsg.checkIsProviderDriven()) {
            BufferHelper.writeArrayAndColon(JSON_PROVIDER_DRIVEN, outBuffer, true, error);
            BufferHelper.writeArray(trueString, outBuffer, false, error);
        }

        if (genericMsg.containerType() != DataTypes.NO_DATA) {
            BufferHelper.comma(outBuffer, error);
            if (!converter.getContainerHandler(genericMsg.containerType()).encodeJson(decIter, outBuffer, true, null, error))
                return false;
        }

        return error.isSuccessful();
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        ((Msg)msg).msgClass(MsgClasses.GENERIC);
        GenericMsg genericMsg = (GenericMsg) msg;
        genericMsg.domainType(DomainTypes.MARKET_PRICE);
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
                        genericMsg.streamId(getInt(currentNode, key, error));
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
                            genericMsg.domainType(
                                    ConstCharArrays.JsonDomain.ofValue(
                                            getText(currentNode, key, error)
                                            , error)
                            );
                        else
                            genericMsg.domainType(getInt(currentNode, key, error));
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
                        genericMsg.containerType(converter.getContainerDataType(key, currentNode, error));
                        break;

                    case JSON_KEY:
                        genericMsg.applyHasMsgKey();
                        converter.decodeChunk(RsslMsgChunkType.MSG_KEY_CHUNK, currentNode, genericMsg, error);
                        break;
                    case JSON_EXTHDR:
                        genericMsg.applyHasExtendedHdr();
                        extendedHeaderBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), extendedHeaderBuffer, error);
                        if (error.isFailed())
                            break;

                        genericMsg.extendedHeader(extendedHeaderBuffer);
                        break;


                    case JSON_REQKEY:
                        //not yet supported by ETAJ, ignore for now
                        break;

                    case JSON_COMPLETE:
                        isComplete = getBoolean(currentNode, key, error);
                        break;

                    case JSON_PARTNUMBER:
                        genericMsg.applyHasPartNum();
                        genericMsg.partNum(getInt(currentNode, key, error));
                        break;

                    case JSON_PERMDATA:
                        genericMsg.applyHasPermData();
                        permDataBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), permDataBuffer, error);
                        if (error.isFailed())
                            break;

                        genericMsg.permData(permDataBuffer);
                        break;

                    case JSON_SEQNUM:
                        genericMsg.applyHasSeqNum();
                        genericMsg.seqNum(getLong(currentNode, key, error));
                        break;

                    case JSON_SECSEQNUM:
                        genericMsg.applyHasSecondarySeqNum();
                        genericMsg.secondarySeqNum(getLong(currentNode, key, error));
                        break;

                    case JSON_PROVIDER_DRIVEN:
                        genericMsg.applyProviderDriven();
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
                genericMsg.applyMessageComplete();

        } finally {
            if (extendedHeaderBuffer != null)
                JsonFactory.releaseBuffer(extendedHeaderBuffer);

            if (permDataBuffer != null)
                JsonFactory.releaseBuffer(permDataBuffer);
        }

    }
}
