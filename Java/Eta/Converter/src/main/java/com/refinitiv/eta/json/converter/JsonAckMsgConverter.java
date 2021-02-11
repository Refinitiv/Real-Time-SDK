package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;
import com.refinitiv.eta.rdm.DomainTypes;

import java.util.Iterator;
import java.util.Objects;

import static com.refinitiv.eta.json.converter.BufferConverter.decodeFromBase64;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonAckMsgConverter extends AbstractRsslMessageTypeConverter {
    JsonAckMsgConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    @Override
    boolean encodeJson(DecodeIterator decIter, Object msg, JsonBuffer outBuffer, JsonConverterError error) {

        AckMsg ackMsg = (AckMsg) msg;
        BufferHelper.writeArrayAndColon(JSON_ACKID, outBuffer, true, error);
        BasicPrimitiveConverter.writeLong(ackMsg.ackId(), outBuffer, error);
        if (ackMsg.flags() != AckMsgFlags.NONE) {
            if (ackMsg.checkHasNakCode()) {
                BufferHelper.writeArrayAndColon(JSON_NAKCODE, outBuffer, true, error);
                String code = getNakCodeString(ackMsg.nakCode());
                if (code != null)
                    BufferHelper.writeArray(code, outBuffer, true, error);
                else
                    BasicPrimitiveConverter.writeLong(ackMsg.nakCode(), outBuffer, error);
            }

            if (ackMsg.checkHasText()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_TEXT, outBuffer, true, error);
                BasicPrimitiveConverter.writeSafeString(ackMsg.text(), outBuffer, error);
            }

            if (ackMsg.checkPrivateStream()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PRIVATE, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }

            if (ackMsg.checkHasSeqNum()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_SEQNUM, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(ackMsg.seqNum(), outBuffer, error);
            }

            if (ackMsg.checkHasMsgKey()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEY, outBuffer, true, error);
                if (!converter.processMsgKey(decIter, ackMsg.msgKey(), outBuffer, ackMsg.domainType(), true, error))
                    return false;
            }

            if (ackMsg.checkHasExtendedHdr()) {
                if (Objects.nonNull(ackMsg.extendedHeader()) && Objects.nonNull(ackMsg.extendedHeader().data())) {
                    BufferHelper.writeArrayAndColon(JSON_EXTHDR, outBuffer, true, error);
                    if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(ackMsg.extendedHeader(), outBuffer, error))
                        return false;
                } else {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Found empty extended header");
                    return false;
                }
            }

            if (ackMsg.checkQualifiedStream()) {
                BufferHelper.writeArrayAndColon(JSON_QUALIFIED, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }
        }

        if (ackMsg.containerType() != DataTypes.NO_DATA) {
            BufferHelper.comma(outBuffer, error);
            if (!converter.getContainerHandler(ackMsg.containerType()).encodeJson(decIter, outBuffer, true, null, error))
                return false;
        }

        return error.isSuccessful();
    }

    private String getNakCodeString(int code) {

        switch (code) {
            case NakCodes.NONE:
                return ConstCharArrays.NAKC_NONE;
            case NakCodes.ACCESS_DENIED:
                return ConstCharArrays.NAKC_ACCESS_DENIED;
            case NakCodes.DENIED_BY_SRC:
                return ConstCharArrays.NAKC_DENIED_BY_SRC;
            case NakCodes.SOURCE_DOWN:
                return ConstCharArrays.NAKC_SOURCE_DOWN;
            case NakCodes.SOURCE_UNKNOWN:
                return ConstCharArrays.NAKC_SOURCE_UNKNOWN;
            case NakCodes.NO_RESOURCES:
                return ConstCharArrays.NAKC_NO_RESOURCES;
            case NakCodes.NO_RESPONSE:
                return ConstCharArrays.NAKC_NO_RESPONSE;
            case NakCodes.GATEWAY_DOWN:
                return ConstCharArrays.NAKC_GATEWAY_DOWN;
            case NakCodes.SYMBOL_UNKNOWN:
                return ConstCharArrays.NAKC_SYMBOL_UNKNOWN;
            case NakCodes.NOT_OPEN:
                return ConstCharArrays.NAKC_NOT_OPEN;
            case NakCodes.INVALID_CONTENT:
                return ConstCharArrays.NAKC_INVALID_CONTENT;
            default:
                return null;
        }
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        ((Msg)msg).msgClass(MsgClasses.ACK);
        AckMsg ackMsg = (AckMsg) msg;
        ackMsg.domainType(DomainTypes.MARKET_PRICE);
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
                        ackMsg.streamId(getInt(currentNode, key, error));
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
                            ackMsg.domainType(
                                    ConstCharArrays.JsonDomain.ofValue(
                                            getText(currentNode, key, error)
                                            , error)
                            );
                        else
                            ackMsg.domainType(getInt(currentNode, key, error));
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
                        ackMsg.containerType(converter.getContainerDataType(key, currentNode, error));
                        break;

                    case JSON_KEY:
                        ackMsg.applyHasMsgKey();
                        converter.decodeChunk(RsslMsgChunkType.MSG_KEY_CHUNK, currentNode, ackMsg, error);
                        break;

                    case JSON_SEQNUM:
                        ackMsg.applyHasSeqNum();
                        ackMsg.seqNum(getLong(currentNode, key, error));
                        break;

                    case JSON_TEXT:
                        ackMsg.applyHasText();
                        ackMsg.text().data(getText(currentNode, key, error));
                        break;

                    case JSON_ACKID:
                        ackMsg.ackId(getLong(currentNode, key, error));
                        break;

                    case JSON_PRIVATE:
                        if (getBoolean(currentNode, key, error))
                            ackMsg.applyPrivateStream();
                        break;

                    case JSON_QUALIFIED:
                        if (getBoolean(currentNode, key, error))
                            ackMsg.applyQualifiedStream();
                        break;

                    case JSON_EXTHDR:
                        ackMsg.applyHasExtendedHdr();
                        extendedHeaderBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), extendedHeaderBuffer, error);
                        if (error.isFailed())
                            break;

                        ackMsg.extendedHeader(extendedHeaderBuffer);
                        break;

                    case JSON_NAKCODE:
                        checkStringOrInt(currentNode, key, error);
                        if (error.isFailed())
                            return;

                        ackMsg.applyHasNakCode();
                        if (currentNode.isTextual())
                            ackMsg.nakCode(ConstCharArrays.JsonNackCode.ofValue(getText(currentNode, key, error), error));
                        else
                            ackMsg.nakCode(getInt(currentNode, key, error));
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
}
