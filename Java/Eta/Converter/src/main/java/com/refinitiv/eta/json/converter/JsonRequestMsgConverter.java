package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.ViewTypes;

import java.util.Iterator;
import java.util.Objects;

import static com.refinitiv.eta.json.converter.BufferConverter.decodeFromBase64;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;

class JsonRequestMsgConverter extends AbstractRsslMessageTypeConverter {

    JsonRequestMsgConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    @Override
    protected void decodeJson(JsonNode node, Object msg, JsonConverterError error) {
        RequestMsg requestMsg = (RequestMsg) msg;
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        //use flags to set properties with default True value
        boolean keyInUpdates = true;
        boolean isStreaming = true;

        boolean hasId = false;
        boolean hasKey = false;

        Buffer extendedHeaderBuffer = null;

        try {
            for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
                String key = it.next();
                JsonNode currentNode = node.path(key);
                switch (key) {
                    case JSON_CONFINFOINUPDATES:
                        if (getBoolean(currentNode, key, error))
                            requestMsg.applyConfInfoInUpdates();
                        break;
                    case JSON_ID:
                        requestMsg.streamId(getInt(currentNode, key, error));
                        hasId = true;
                        break;
                    case JSON_TYPE:
                        //message type already considered on top level
                        break;

                    case JSON_DOMAIN:
                        if (currentNode.isTextual())
                            requestMsg.domainType(
                                    ConstCharArrays.JsonDomain.ofValue(
                                            getText(currentNode, key, error)
                                            , error)
                            );
                        else
                            requestMsg.domainType(getInt(currentNode, key, error));
                        break;

                    case JSON_EXTHDR:
                        requestMsg.applyHasExtendedHdr();

                        extendedHeaderBuffer = JsonFactory.createBuffer();
                        decodeFromBase64(getText(currentNode, key, error), extendedHeaderBuffer, error);
                        if (error.isFailed())
                            break;

                        requestMsg.extendedHeader(extendedHeaderBuffer);
                        break;

                    //treat as body
                    case JSON_ELEMENTS:
                    case JSON_FIELDS:
                    case JSON_FILTERLIST:
                    case JSON_MAP:
                    case JSON_MESSAGE:
                    case JSON_JSON:
                    case JSON_XML:
                    case JSON_OPAQUE:
                    case JSON_SERIES:
                        requestMsg.containerType(converter.getContainerDataType(key, currentNode, error));
                        break;

                    case JSON_KEY:
                       // requestMsg.applyMsgKeyInUpdates();
                        converter.decodeChunk(RsslMsgChunkType.MSG_KEY_CHUNK, currentNode, requestMsg, error);
                        hasKey = true;
                        break;
                    case JSON_KEYINUPDATES:
                        keyInUpdates = getBoolean(currentNode, key, error);
                        break;
                    case JSON_VIEW:
                        requestMsg.applyHasView();
                        requestMsg.containerType(DataTypes.ELEMENT_LIST);
                        break;
                    case JSON_STREAMING:
                        isStreaming = getBoolean(currentNode, key, error);
                        break;
                    case JSON_REFRESH:
                        if (!getBoolean(currentNode, key, error))
                            requestMsg.applyNoRefresh();
                        break;
                    case JSON_PRIVATE:
                        if (getBoolean(currentNode, key, error))
                            requestMsg.applyPrivateStream();
                        break;
                    case JSON_PAUSE:
                        if (getBoolean(currentNode, key, error))
                            requestMsg.applyPause();
                        break;
                    case JSON_PRIORITY:
                        requestMsg.applyHasPriority();
                        converter.decodeChunk(RsslMsgChunkType.PRIORITY_CHUNK, currentNode, requestMsg, error);
                        break;
                    case JSON_QOS:
                        requestMsg.applyHasQos();
                        converter.decodeChunk(DataTypes.QOS, currentNode, requestMsg.qos(), error);
                        break;
                    case JSON_QUALIFIED:
                        if (getBoolean(currentNode, key, error))
                            requestMsg.applyQualifiedStream();
                        break;
                    case JSON_WORSTQOS:
                        requestMsg.applyHasWorstQos();
                        converter.decodeChunk(DataTypes.QOS, currentNode, requestMsg.worstQos(), error);
                        break;
                    default:
                        processUnexpectedKey(key, error);
                }

                if (error.isFailed())
                    return;
            }

            if (keyInUpdates)
                requestMsg.applyMsgKeyInUpdates();

            if (isStreaming)
                requestMsg.applyStreaming();

            if (!hasKey)
                error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, JSON_KEY);

            if (!hasId)
                error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, JSON_ID);

            setDeafultDynamicQoS(requestMsg);
        } finally {
            if (extendedHeaderBuffer != null)
                JsonFactory.releaseBuffer(extendedHeaderBuffer);
        }
    }

    @Override
    boolean encodeJson(DecodeIterator decIter, Object msg, JsonBuffer outBuffer, JsonConverterError error) {

        boolean comma = false;

        RequestMsg reqMsg = (RequestMsg) msg;

        if (!reqMsg.checkStreaming()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_STREAMING, outBuffer, true, error);
            BufferHelper.writeArray(ConstCharArrays.falseString, outBuffer, false, error);
        }

        if (reqMsg.flags() != RequestMsgFlags.NONE) {
            if (reqMsg.checkHasPriority()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PRIORITY, outBuffer, true, error);
                BufferHelper.beginObject(outBuffer, error);
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_CLASS, outBuffer, false, error);
                BasicPrimitiveConverter.writeLong(reqMsg.priority().priorityClass(), outBuffer, error);
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_COUNT, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(reqMsg.priority().count(), outBuffer, error);
                BufferHelper.endObject(outBuffer, error);
            }

            if (!reqMsg.checkMsgKeyInUpdates()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEYINUPDATES, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.falseString, outBuffer, false, error);
            }

            if (reqMsg.checkConfInfoInUpdates()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_CONFINFOINUPDATES, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }

            if (reqMsg.checkNoRefresh()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_REFRESH, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.falseString, outBuffer, false, error);
            }

            if (reqMsg.checkHasQos()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_QOS, outBuffer, true, error);
                if (!((JsonQosConverter) converter.getPrimitiveHandler(DataTypes.QOS)).writeToJson(reqMsg.qos(), outBuffer, error))
                    return false;
            }

            if (reqMsg.checkHasWorstQos()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_WORSTQOS, outBuffer, true, error);
                if (!((JsonQosConverter) converter.getPrimitiveHandler(DataTypes.QOS)).writeToJson(reqMsg.worstQos(), outBuffer, error))
                    return false;
            }

            if (reqMsg.checkPrivateStream()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PRIVATE, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }

            if (reqMsg.checkQualifiedStream()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_QUALIFIED, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }

            if (reqMsg.checkPause()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PAUSE, outBuffer, true, error);
                BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            }

            if (reqMsg.checkHasExtendedHdr()) {
                if (Objects.nonNull(reqMsg.extendedHeader()) && Objects.nonNull(reqMsg.extendedHeader().data())) {
                    BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_EXTHDR, outBuffer, true, error);
                    if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(reqMsg.extendedHeader(), outBuffer, error))
                        return false;
                } else {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Empty extended header found");
                    return false;
                }
            }
        }

        if (!reqMsg.checkHasBatch()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEY, outBuffer, true, error);
            if (!converter.processMsgKey(decIter, reqMsg.msgKey(), outBuffer, reqMsg.domainType(), true, error))
                return false;
            if (!reqMsg.checkHasView()) {
                if (reqMsg.containerType() != DataTypes.NO_DATA) {
                    BufferHelper.comma(outBuffer, error);
                    if (!converter.getContainerHandler(reqMsg.containerType()).encodeJson(decIter, outBuffer, true, null, error))
                        return false;
                }
                return true;
            }
        }

        if (reqMsg.containerType() != DataTypes.ELEMENT_LIST || reqMsg.encodedDataBody().length() == 0) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Batch/View request: expected nonempty ElementList data body, found: dataType = "
                    +  reqMsg.containerType() + ", length: " + reqMsg.encodedDataBody().length());
            return false;
        }

        ElementList elemList = JsonFactory.createElementList();
        ElementEntry elemEntry = JsonFactory.createElementEntry();
        UInt uInt = JsonFactory.createUInt();
        Int iv = JsonFactory.createInt();
        Array array = JsonFactory.createArray();
        ArrayEntry arrayElem = JsonFactory.createArrayEntry();
        DecodeIterator arrayIter = JsonFactory.createDecodeIterator();

        Buffer viewDataBuf = JsonFactory.createBuffer();
        Buffer itemListBuf = JsonFactory.createBuffer();
        Buffer viewTypeBuf = JsonFactory.createBuffer();

        itemListBuf.clear();
        viewDataBuf.clear();
        viewTypeBuf.clear();

        int ret = 0;
        elemList.clear();
        elemEntry.clear();

        try {

            ret = elemList.decode(decIter, null);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed decoding ElementList, code = " + ret);
                return false;
            }
            ret = elemEntry.decode(decIter);
            while (ret != CodecReturnCodes.END_OF_CONTAINER) {
                if (ret < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed decoding ElementEntry, code = " + ret);
                    return false;
                } else {
                    if (ElementNames.VIEW_TYPE.equals(elemEntry.name())) {
                        uInt.decode(decIter);
                        viewTypeBuf.data(elemEntry.encodedData().data(), elemEntry.encodedData().position(), elemEntry.encodedData().length());
                    } else if (ElementNames.VIEW_DATA.equals(elemEntry.name())) {
                        viewDataBuf.data(elemEntry.encodedData().data(), elemEntry.encodedData().position(), elemEntry.encodedData().length());
                    } else if (ElementNames.BATCH_ITEM_LIST.equals(elemEntry.name())) {
                        itemListBuf.data(elemEntry.encodedData().data(), elemEntry.encodedData().position(), elemEntry.encodedData().length());
                    }
                }
                elemEntry.clear();
                ret = elemEntry.decode(decIter);
            }

            if (reqMsg.checkHasBatch()) {
                if (itemListBuf.length() == 0) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed decoding Request message: batch data missing");
                    return false;
                }
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEY, outBuffer, true, error);
                if (!converter.processMsgKey(decIter, reqMsg.msgKey(), outBuffer, reqMsg.domainType(), true, error))
                    return false;

                outBuffer.position--; //remove closing bracket

                arrayIter.clear();
                arrayIter.setBufferAndRWFVersion(itemListBuf, Codec.majorVersion(), Codec.minorVersion());
                array.clear();
                ret = array.decode(arrayIter);
                if (ret < CodecReturnCodes.SUCCESS || ret == CodecReturnCodes.BLANK_DATA || ret == CodecReturnCodes.NO_DATA) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Expected array data, return code = " + ret);
                    return false;
                }

                if (outBuffer.data[outBuffer.position - 1] != '{')
                    BufferHelper.comma(outBuffer, error);

                comma = false;
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_NAME, outBuffer, false, error);
                BufferHelper.beginArray(outBuffer, error);
                arrayElem.clear();
                ret = arrayElem.decode(arrayIter);
                arrayIter.setBufferAndRWFVersion(itemListBuf, Codec.majorVersion(), Codec.minorVersion());
                while (ret != CodecReturnCodes.END_OF_CONTAINER) {
                    if (ret < CodecReturnCodes.SUCCESS) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed to decode array entry, return code = " + ret);
                        return false;
                    } else {
                        if (comma)
                            BufferHelper.comma(outBuffer, error);
                        else
                            comma = true;
                        BasicPrimitiveConverter.writeAsciiString(arrayElem.encodedData(), outBuffer, error);
                    }
                    arrayElem.clear();
                    ret = arrayElem.decode(arrayIter);
                }
                BufferHelper.endArray(outBuffer, error);
                BufferHelper.endObject(outBuffer, error); //add key closing bracket
            }

            if (viewDataBuf.length() == 0)
                return true;

            if (viewTypeBuf.length() == 0) {
                if (error.getCode() == CodecReturnCodes.SUCCESS)
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Missing view type entry");
                return false;
            }

            if (uInt.toLong() == ViewTypes.ELEMENT_NAME_LIST) {
                BufferHelper.writeArrayAndColon(JSON_VIEWTYPE, outBuffer, true, error);
                BufferHelper.writeArray(JSON_VIEW_NAME_LIST, outBuffer, true, error);
            } else if (uInt.toLong() == ViewTypes.FIELD_ID_LIST) {
                BufferHelper.writeArrayAndColon(JSON_VIEWTYPE, outBuffer, true, error);
                BufferHelper.writeArray(JSON_VIEW_FID_LIST, outBuffer, true, error);
            }

            comma = false;
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_VIEW, outBuffer, true, error);
            BufferHelper.beginArray(outBuffer, error);
            arrayElem.clear();
            arrayIter.clear();
            arrayIter.setBufferAndRWFVersion(viewDataBuf, Codec.majorVersion(), Codec.minorVersion());
            array.clear();
            ret = array.decode(arrayIter);
            if (ret < CodecReturnCodes.SUCCESS || ret == CodecReturnCodes.BLANK_DATA || ret == CodecReturnCodes.NO_DATA) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Expected array data, return code = " + ret);
                return false;
            }
            ret = arrayElem.decode(arrayIter);
            while (ret != CodecReturnCodes.END_OF_CONTAINER) {
                if (ret < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed to decode array entry, return code = " + ret);
                    return false;
                } else {
                    if (comma)
                        BufferHelper.comma(outBuffer, error);
                    else
                        comma = true;

                    if (uInt.toLong() == ViewTypes.ELEMENT_NAME_LIST)
                        BasicPrimitiveConverter.writeAsciiString(arrayElem.encodedData(), outBuffer, error);
                    else {
                        iv.clear();
                        iv.decode(arrayIter);
                        BasicPrimitiveConverter.writeLong(iv.toLong(), outBuffer, error);
                    }
                }
                arrayElem.clear();
                ret = arrayElem.decode(arrayIter);
            }
            BufferHelper.endArray(outBuffer, error);

        } finally {
            JsonFactory.releaseBuffer(viewDataBuf);
            JsonFactory.releaseBuffer(itemListBuf);
            JsonFactory.releaseBuffer(viewTypeBuf);
            JsonFactory.releaseDecodeIterator(arrayIter);
            JsonFactory.releaseArray(array);
            JsonFactory.releaseArrayEntry(arrayElem);
        }

        return true;
    }
}
