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
import com.refinitiv.eta.rdm.ElementNames;

import java.util.Iterator;
import java.util.Objects;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.BufferConverter.decodeFromBase64;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonCloseMsgConverter extends AbstractRsslMessageTypeConverter {
    JsonCloseMsgConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    @Override
    boolean encodeJson(DecodeIterator decIter, Object msg, JsonBuffer outBuffer, JsonConverterError error) {

        CloseMsg closeMsg = (CloseMsg) msg;
        boolean isBatch = false;
        JsonBuffer elementListBuffer = null;
        JsonBuffer batchBuffer = null;

        if (closeMsg.containerType() == DataTypes.ELEMENT_LIST && closeMsg.encodedDataBody().length() != 0) {
            elementListBuffer = new JsonBuffer();
            batchBuffer = new JsonBuffer();
            if (!decodeElementList(batchBuffer, elementListBuffer, decIter, estimateJsonLength(closeMsg.encodedDataBody().length()), error))
                return false;
            if (batchBuffer.data != null)
                isBatch = true;
        }

        BufferHelper.writeArrayAndColon(JSON_ID, outBuffer, false, error);
        if (isBatch)
            BufferHelper.copyToByteArray(batchBuffer.data, 0, batchBuffer.position, outBuffer, error);
        else
            BasicPrimitiveConverter.writeLong(closeMsg.streamId(), outBuffer, error);

        String msgType = getMsgClassString(closeMsg.msgClass());
        BufferHelper.writeArrayAndColon(JSON_TYPE, outBuffer, true, error);
        if (msgType != null)
            BufferHelper.writeArray(msgType, outBuffer, true, error);
        else
            BasicPrimitiveConverter.writeLong(closeMsg.msgClass(), outBuffer, error);

        if (closeMsg.domainType() != DomainTypes.MARKET_PRICE) {
            String domainType = getDomainString(closeMsg.domainType());
            BufferHelper.writeArrayAndColon(JSON_DOMAIN, outBuffer, true, error);
            if (domainType != null)
                BufferHelper.writeArray(domainType, outBuffer, true, error);
            else
                BasicPrimitiveConverter.writeLong(closeMsg.domainType(), outBuffer, error);
        }

        if (closeMsg.checkAck()) {
            BufferHelper.writeArrayAndColon(JSON_ACK, outBuffer, true, error);
            BufferHelper.writeArray(trueString, outBuffer, false, error);
        }

        if (closeMsg.checkHasExtendedHdr()) {
            if (Objects.nonNull(closeMsg.extendedHeader()) && Objects.nonNull(closeMsg.extendedHeader().data())) {
                BufferHelper.writeArrayAndColon(JSON_EXTHDR, outBuffer, true, error);
                if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(closeMsg.extendedHeader(), outBuffer, error))
                    return false;
            } else {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Empty extended header found");
                return false;
            }
        }

        if (!isBatch && closeMsg.containerType() != DataTypes.NO_DATA) {
            if (closeMsg.containerType() == DataTypes.ELEMENT_LIST) {
                if (elementListBuffer.data != null) {
                    BufferHelper.writeArrayAndColon(JSON_ENTRIES, outBuffer, true, error);
                    BufferHelper.beginArray(outBuffer, error);
                    BufferHelper.copyToByteArray(elementListBuffer.data, 0, elementListBuffer.position, outBuffer, error);
                    BufferHelper.endArray(outBuffer, error);
                }
            } else {
                BufferHelper.comma(outBuffer, error);
                if (!converter.getContainerHandler(closeMsg.containerType()).encodeJson(decIter, outBuffer, true, null, error))
                    return false;
            }
        }

        return error.isSuccessful();
    }

    private int estimateJsonLength(int rwfLength) {
        return rwfLength * 6 + 300;
    }

    private boolean decodeElementList(JsonBuffer batchStreamIdBuffer, JsonBuffer elementListBuffer, DecodeIterator decIter, int estimatedElementListLength, JsonConverterError error) {

        ElementList elemList = JsonFactory.createElementList();
        ElementEntry elemEntry = JsonFactory.createElementEntry();
        try {
            elemList.clear();
            int ret = elemList.decode(decIter, null);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed decoding ElementList payload, code = " + ret);
                return false;
            }
            boolean listComma = false;
            boolean listRes = true;
            while ((ret = elemEntry.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER) {
                if (ret < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed decoding ElementEntry of payload, code = " + ret);
                    return false;
                } else {
                    if (elemEntry.name().equals(ElementNames.BATCH_STREAMID_LIST)) {
                        return getBatchStreamIds(batchStreamIdBuffer, decIter, estimateJsonLength(elemEntry.encodedData().length()), error); //C version skips all other entries even if they are present in the payload in case there is batch request
                    } else {
                        if (elementListBuffer.data == null)
                            elementListBuffer.data = new byte[estimatedElementListLength];
                        if (listComma)
                            listRes = listRes && BufferHelper.comma(elementListBuffer, error);
                        else
                            listComma = true;
                        listRes = listRes && converter.getContainerHandler(DataTypes.ELEMENT_LIST).writeEntry(decIter, elementListBuffer, null, error, elemEntry, elemList);
                    }
                }
                elemEntry.clear();
                elemEntry.decode(decIter);
            }

            return listRes;
        } finally {
            JsonFactory.releaseElementList(elemList);
            JsonFactory.releaseElementEntry(elemEntry);
        }
    }

    private boolean getBatchStreamIds(JsonBuffer batchStreamIdBuffer, DecodeIterator decIter, int estimatedLength, JsonConverterError error) {
        Array array = JsonFactory.createArray();
        ArrayEntry arrayEntry = JsonFactory.createArrayEntry();
        Int fid = CodecFactory.createInt();
        try {
            array.clear();
            int ret = array.decode(decIter);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed decoding Array, code = " + ret);
                return false;
            }
            batchStreamIdBuffer.data = new byte[estimatedLength];
            boolean res = BufferHelper.beginArray(batchStreamIdBuffer, error);
            boolean comma = false;
            ret = arrayEntry.decode(decIter);
            while (res && ret != CodecReturnCodes.END_OF_CONTAINER) {
                if (ret < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed decoding ArrayEntry, code = " + ret);
                    return false;
                } else {
                    if (comma)
                        res = BufferHelper.comma(batchStreamIdBuffer, error);
                    else
                        comma = true;
                    fid.clear();
                    ret = fid.decode(decIter);
                    if (ret < CodecReturnCodes.SUCCESS) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed decoding Int, code = " + ret);
                        return false;
                    }
                    res = res && BasicPrimitiveConverter.writeLong(fid.toLong(), batchStreamIdBuffer, error);
                }
                arrayEntry.clear();
                ret = arrayEntry.decode(decIter);
            }
            return res && BufferHelper.endArray(batchStreamIdBuffer, error);

        } finally {
            JsonFactory.releaseArrayEntry(arrayEntry);
            JsonFactory.releaseArray(array);
            JsonFactory.releaseInt(fid);
        }
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
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        ((Msg)msg).msgClass(MsgClasses.CLOSE);
        CloseMsg closeMsg = (CloseMsg) msg;
        closeMsg.domainType(DomainTypes.MARKET_PRICE);
        //use flags to set properties with default True value

        boolean hasId = false;
        boolean batchClose = false;

        Buffer extendedHeaderBuffer = null;
        Buffer permDataBuffer = null;

        try {
            for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
                String key = it.next();
                JsonNode currentNode = node.path(key);
                switch (key) {
                    case JSON_ID:
                        if (currentNode.isInt()) {
                            closeMsg.streamId(getInt(currentNode, key, error));
                            hasId = true;
                        }
                        else if (currentNode.isArray()) {
                            if (currentNode.get(0).isInt())
                                closeMsg.streamId(currentNode.get(0).asInt());
                            else
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected integer for IDs array, found " + currentNode.getNodeType().toString());
                            batchClose = true;
                        } else
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected integer or array type for ID, found " + currentNode.getNodeType().toString());
                        break;
                    case JSON_TYPE:
                        //message type already considered on top level
                        break;

                    case JSON_DOMAIN:
                        checkStringOrInt(currentNode, key, error);
                        if (error.isFailed())
                            break;

                        if (currentNode.isTextual())
                            closeMsg.domainType(
                                    ConstCharArrays.JsonDomain.ofValue(
                                            getText(currentNode, key, error)
                                            , error)
                            );
                        else
                            closeMsg.domainType(getInt(currentNode, key, error));
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
                        if (!batchClose)
                            closeMsg.containerType(converter.getContainerDataType(key, currentNode, error));
                        break;

                    case JSON_ACK:
                        if (getBoolean(currentNode, key, error))
                            closeMsg.applyAck();
                        break;


                    case JSON_EXTHDR:
                        closeMsg.applyHasExtendedHdr();
                        extendedHeaderBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), extendedHeaderBuffer, error);
                        if (error.isFailed())
                            break;

                        closeMsg.extendedHeader(extendedHeaderBuffer);
                        break;

                    default:
                        processUnexpectedKey(key, error);
                }

                if (error.isFailed())
                    return;
            }

            if (!hasId && !batchClose)
                error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, JSON_ID);
            if (batchClose)
                closeMsg.containerType(DataTypes.ELEMENT_LIST);

        } finally {
            if (extendedHeaderBuffer != null)
                JsonFactory.releaseBuffer(extendedHeaderBuffer);

            if (permDataBuffer != null)
                JsonFactory.releaseBuffer(permDataBuffer);
        }
    }

    @Override
    void encodeRWF(JsonNode dataNode, String key, Msg msg, EncodeIterator iter, JsonConverterError error) {

        int result = SUCCESS;

        JsonNode idNode = dataNode.path(JSON_ID);
        if (idNode.isInt()) {
            super.encodeRWF(dataNode, key, msg, iter, error);
        } else if (idNode.isArray() && idNode.size() > 0) {
            msg.containerType(DataTypes.ELEMENT_LIST);
            result = msg.encodeInit(iter, 0);
            if (result < SUCCESS) {
                error.setEncodeError(result, "msg [" + msg.msgClass() + "] encode");
                return;
            }
            ElementList elemList = JsonFactory.createElementList();
            ElementEntry elemEntry = JsonFactory.createElementEntry();
            Array array = JsonFactory.createArray();
            ArrayEntry arrayEntry = JsonFactory.createArrayEntry();
            Int iv = JsonFactory.createInt();

            elemList.clear();
            elemEntry.clear();
            array.clear();
            arrayEntry.clear();


            try {
                elemList.applyHasStandardData();
                result = elemList.encodeInit(iter, null, 0);
                if (result < SUCCESS) {
                    error.setEncodeError(result, "msg [" + msg.msgClass() + "] encode");
                    return;
                }
                elemEntry.name(ElementNames.BATCH_STREAMID_LIST);
                elemEntry.dataType(DataTypes.ARRAY);
                result = elemEntry.encodeInit(iter, 0);
                if (result < SUCCESS) {
                    error.setEncodeError(result, "msg [" + msg.msgClass() + "] encode");
                    return;
                }
                array.primitiveType(DataTypes.INT);
                result = array.encodeInit(iter);
                if (result < SUCCESS) {
                    error.setEncodeError(result, "msg [" + msg.msgClass() + "] encode");
                    return;
                }
                for (int i = 0; i < idNode.size(); i++) {
                    iv.clear();
                    converter.getPrimitiveHandler(DataTypes.INT).decodeJson(idNode.get(i), iv, error);
                    if (error.isFailed())
                        return;
                    arrayEntry.clear();
                    result = arrayEntry.encode(iter, iv);
                    if (result < SUCCESS) {
                        error.setEncodeError(result, "msg [" + msg.msgClass() + "] encode");
                        return;
                    }
                }
                result = array.encodeComplete(iter, true);
                if (result < SUCCESS) {
                    error.setEncodeError(result, "msg [" + msg.msgClass() + "] encode");
                    return;
                }
                result = elemEntry.encodeComplete(iter, true);
                if (result < SUCCESS) {
                    error.setEncodeError(result, "msg [" + msg.msgClass() + "] encode");
                    return;
                }
                result = elemList.encodeComplete(iter, true);
                if (result < SUCCESS) {
                    error.setEncodeError(result, "msg [" + msg.msgClass() + "] encode");
                    return;
                }

            } finally {
                JsonFactory.releaseElementList(elemList);
                JsonFactory.releaseElementEntry(elemEntry);
                JsonFactory.releaseArrayEntry(arrayEntry);
                JsonFactory.releaseArray(array);
                JsonFactory.releaseInt(iv);
            }

            result = msg.encodeComplete(iter, error.isSuccessful());
        }

        if (error.isFailed())
            return;

        if (result < SUCCESS) {
            error.setEncodeError(result, "msg encode complete");
            return;
        }
    }

}
