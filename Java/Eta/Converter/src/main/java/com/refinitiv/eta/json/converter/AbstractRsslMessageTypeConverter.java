package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.ViewTypes;

import java.util.Objects;

import static com.refinitiv.eta.codec.CodecReturnCodes.*;

abstract class AbstractRsslMessageTypeConverter extends AbstractTypeConverter {
    AbstractRsslMessageTypeConverter(JsonAbstractConverter converter) {
        super(converter);
    }

    abstract boolean encodeJson(DecodeIterator decIter, Object msg, JsonBuffer outBuffer, JsonConverterError error);

    void encodeRWF(JsonNode dataNode, String key, Msg msg, EncodeIterator iter, JsonConverterError error) {

        int result = SUCCESS;

        if (msg.containerType() == NO_DATA) {
            result = msg.encode(iter);
            if (result != SUCCESS)
                error.setEncodeError(result, "Message type [" + msg.msgClass() + "]");
            return;
        }

        result = msg.encodeInit(iter, 0);
        if (result < SUCCESS) {
            error.setEncodeError(result, "msg [" + msg.msgClass() + "] encode");
            return;
        }

        if (result == ENCODE_MSG_KEY_ATTRIB) {
            encodeMsgAttribs(dataNode, iter, error);

            result = msg.encodeKeyAttribComplete(iter, error.isSuccessful());

            if (error.isFailed())
                return;

            if (result < SUCCESS) {
                error.setEncodeError(result, "msg attrib");
                return;
            }
        }

        if (result == ENCODE_CONTAINER) {
            encodeMsgPayload(dataNode, msg, iter, error);
        }

        result = msg.encodeComplete(iter, error.isSuccessful());

        if (error.isFailed())
            return;

        if (result < SUCCESS) {
            error.setEncodeError(result, "msg encode complete");
            return;
        }
    }

    void encodeMsgAttribs(JsonNode node, EncodeIterator iter, JsonConverterError error)
    {
        String key = ConstCharArrays.JSON_ELEMENTS;
        JsonNode currentNode = node.path(ConstCharArrays.JSON_KEY).path(ConstCharArrays.JSON_ELEMENTS);
        if (currentNode == null) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_PARSE_ERROR, "json key " + key + " ARRAY was not found");
            return;
        }

        converter.decodeChunk(DataTypes.ELEMENT_LIST, currentNode, key, iter, error);
    }

    void encodeMsgPayload(JsonNode node, Msg outMsg, EncodeIterator iter, JsonConverterError error)
    {
        if ((outMsg.msgClass() == MsgClasses.REQUEST) && (((RequestMsg)outMsg).checkHasBatch() || ((RequestMsg)outMsg).checkHasView())) {
            encodeBatchAndView(node, outMsg, iter, error);
            return; //batch is stored inside body, no additional data could be populated
        }

        //based on container
        JsonNode currentNode = null;
        String key = null;
        switch (outMsg.containerType()) {
            case DataTypes.ELEMENT_LIST:
                key = ConstCharArrays.JSON_ELEMENTS;
                break;
            case DataTypes.FILTER_LIST:
                key = ConstCharArrays.JSON_FILTERLIST;
                break;
            case DataTypes.FIELD_LIST:
                key = ConstCharArrays.JSON_FIELDS;
                break;
            case DataTypes.MAP:
                key = ConstCharArrays.JSON_MAP;
                break;
            case DataTypes.VECTOR:
                key = ConstCharArrays.JSON_VECTOR;
                break;
            case DataTypes.OPAQUE:
                key = ConstCharArrays.JSON_OPAQUE;
                break;
            case DataTypes.XML:
                key = ConstCharArrays.JSON_XML;
                break;
            case DataTypes.SERIES:
                key = ConstCharArrays.JSON_SERIES;
                break;
            case DataTypes.JSON:
                key = ConstCharArrays.JSON_JSON;
                break;
            case DataTypes.MSG:
                key = ConstCharArrays.JSON_MESSAGE;
                break;
            default:
                error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_CONTAINER_TYPE, "body type [" + outMsg.containerType() + "] is not supported");
                return;
        }

        currentNode = node.path(key);
        if (currentNode == null) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, "body, missing key '" + key + "'");
            return;
        }
        converter.decodeChunk(outMsg.containerType(), currentNode, key, iter, error);
    }

    void encodeBatchAndView(JsonNode currentRoot, Msg outMsg, EncodeIterator encodeIterator, JsonConverterError error)
    {
        ElementList elementList = CodecFactory.createElementList();
        elementList.applyHasStandardData();
        int result = elementList.encodeInit(encodeIterator, null, getEncodingMaxSize());
        if (result != SUCCESS) {
            error.setEncodeError(result, "msg encode BatchRequest/View elementList");
            return;
        }

        if (((RequestMsg)outMsg).checkHasBatch()) {
            String key = ConstCharArrays.JSON_NAME;
            JsonNode currentNode = currentRoot.path(ConstCharArrays.JSON_KEY).path(key);
            if (currentNode == null) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_PARSE_ERROR, "json key " + key + " was not found");
                return;
            }
            decodeBatchAndViewRequestNamesElementEntry(currentNode, encodeIterator, key, true, error);
            if (error.isFailed()) {
                elementList.encodeComplete(encodeIterator, false);
                return;
            }
        }


        if (((RequestMsg)outMsg).checkHasView()) {
            String key = ConstCharArrays.JSON_VIEWTYPE;
            JsonNode currentNode = currentRoot.path(key);
            if (!currentNode.isNull() || !currentNode.isMissingNode()) {
                if (currentNode.isTextual()) {
                    ElementEntry entry = JsonFactory.createElementEntry();
                    UInt val = JsonFactory.createUInt();
                    try {
                        entry.name(ElementNames.VIEW_TYPE);
                        entry.dataType(DataTypes.UINT);
                        switch (currentNode.asText()) {
                            case ConstCharArrays.JSON_VIEW_FID_LIST:
                                val.value(ViewTypes.FIELD_ID_LIST);
                                break;
                            case ConstCharArrays.JSON_VIEW_NAME_LIST:
                                val.value(ViewTypes.ELEMENT_NAME_LIST);
                                break;
                            default:
                                break;
                        }
                        entry.encode(encodeIterator, val);
                    } finally {
                        JsonFactory.releaseElementEntry(entry);
                        JsonFactory.releaseUInt(val);
                    }
                } else {

                }
            }

            key = ConstCharArrays.JSON_VIEW;
            currentNode = currentRoot.path(key);
            if (currentNode == null) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_PARSE_ERROR, "json key " + key + " was not found");
                return;
            }
            decodeBatchAndViewRequestNamesElementEntry(currentNode, encodeIterator, key, false, error);
            if (error.isFailed()) {
                elementList.encodeComplete(encodeIterator, false);
                return;
            }
        }

        result = elementList.encodeComplete(encodeIterator, true);
        if (result != SUCCESS) {
            error.setEncodeError(result, "msg encode BatchRequest/View elementList complete");
            return;
        }
    }


    private void decodeBatchAndViewRequestNamesElementEntry(JsonNode currentNode, EncodeIterator iter, String key, boolean isBatch, JsonConverterError error) {
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        Buffer batchElementEntryName = CodecFactory.createBuffer();
        batchElementEntryName.data(isBatch ? ":ItemList" : ":ViewData");
        elementEntry.dataType(DataTypes.ARRAY);
        elementEntry.name(batchElementEntryName);

        int result = elementEntry.encodeInit(iter, getEncodingMaxSize());

        if (result != SUCCESS) {
            error.setEncodeError(result, "batch elementEntry encode failure");
            return;
        }

        decodeBatchRequestNamesArray(currentNode, iter, key, isBatch, error);

        result = elementEntry.encodeComplete(iter, error.isSuccessful());
        if (error.isFailed())
            return;

        if (result != SUCCESS) {
            error.setEncodeError(result, "batch elementEntry encode complete");
            return;
        }
    }

    private void decodeBatchRequestNamesArray(JsonNode currentNode, EncodeIterator iter, String key, boolean isBatch, JsonConverterError error) {
        Array array = CodecFactory.createArray();
        array.primitiveType(isBatch ? DataTypes.ASCII_STRING : DataTypes.INT);
        if (isBatch)
            array.itemLength(0);//array elements length is dynamic

        int result = array.encodeInit(iter);

        if (result != SUCCESS) {
            error.setEncodeError(result, "ARRAY encode");
            return;
        }

        processArrayItems(currentNode, iter, key, isBatch, error);

        result = array.encodeComplete(iter, error.isSuccessful());
        if (error.isFailed())
            return;

        if (result != SUCCESS) {
            error.setEncodeError(result, "ARRAY encode complete");
            return;
        }
    }

    private void processArrayItems(JsonNode currentNode, EncodeIterator iter, String key, boolean isBatch, JsonConverterError error) {
        ArrayEntry arrayEntry = JsonFactory.createArrayEntry();
        Buffer bufText = JsonFactory.createBuffer();
        Int intValue = JsonFactory.createInt();
        intValue.value("");
        try {
            for (int i = 0; i < currentNode.size(); i++) {
                arrayEntry.clear();

                if (isBatch && !currentNode.get(i).isTextual()) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Expected String type, found: " + currentNode.get(i).getNodeType().toString());
                    return;
                }

                if (!isBatch && !(currentNode.get(i).isTextual() || currentNode.get(i).isInt())) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Expected String or Int type, found: " + currentNode.get(i).getNodeType().toString());
                    return;
                }

                int result = -1;
                if (isBatch) {
                    bufText.data(currentNode.get(i).textValue());
                    result = arrayEntry.encode(iter, bufText);
                } else {
                    if (currentNode.get(i).isInt()) {
                        intValue.value(currentNode.get(i).intValue());
                    } else if (Objects.nonNull(converter.getDictionary()) && currentNode.get(i).isTextual()) {
                        DictionaryEntry def = converter.getDictionary().entry(currentNode.get(i).textValue());
                        if (def != null) {
                            intValue.value(def.fid());
                        }
                    }
                    if (!intValue.isBlank()) {
                        result = arrayEntry.encode(iter, intValue);
                    }
                }
                if (result != SUCCESS) {
                    error.setEncodeError(result, "ARRAY, element " + i);
                    return;
                }
            }
        } finally {
            JsonFactory.releaseBuffer(bufText);
            JsonFactory.releaseArrayEntry(arrayEntry);
            JsonFactory.releaseInt(intValue);
        }

    }
}
