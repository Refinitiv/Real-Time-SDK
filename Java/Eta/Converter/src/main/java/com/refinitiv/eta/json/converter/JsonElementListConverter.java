/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.node.JsonNodeType;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

import java.util.Iterator;

import static com.refinitiv.eta.codec.CodecReturnCodes.FAILURE;
import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_DATA;
import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_TYPE;

class JsonElementListConverter extends AbstractContainerTypeConverter {

    JsonElementListConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.ELEMENT_LIST };
    }

    @Override
    Object getContainerObject()  { return JsonFactory.createElementList(); }
    @Override
    Object getEntryObject() { return JsonFactory.createElementEntry(); }

    @Override
    void releaseContainer(Object container) {JsonFactory.releaseElementList((ElementList)container);}
    @Override
    void releaseEntry(Object entry) {JsonFactory.releaseElementEntry((ElementEntry)entry);}

    @Override
    boolean hasEntries(Object container) {
        return (((ElementList) container).encodedEntries() != null && ((ElementList) container).encodedEntries().length() != 0)
                || ((ElementList) container).encodedSetData() != null && ((ElementList) container).encodedSetData().length() != 0;
    }

    @Override
    protected int decodeEntry(DecodeIterator decIter, Object entryObj) {
        ElementEntry entry = (ElementEntry) entryObj;
        entry.clear();
        return entry.decode(decIter);
    }

    @Override
    protected boolean writeContent(DecodeIterator decIter, JsonBuffer outBuffer, JsonConverterError error, Object container) {
        return BufferHelper.beginObject(outBuffer, error) && writeEntries(decIter, outBuffer, false, null, error, false, container) && BufferHelper.endObject(outBuffer, error);
    }

    @Override
    protected int decodeContainer(DecodeIterator decIter, Object setDb, Object container) {
        ElementList list = (ElementList) container;
        list.clear();
        return list.decode(decIter, (LocalElementSetDefDb) setDb);
    }

    @Override
    protected boolean writeEntry(DecodeIterator decIter, JsonBuffer outBuffer, Object localSetDb, JsonConverterError error, Object entryObj, Object container) {

        ElementEntry elEntry = (ElementEntry) entryObj;
        if ((elEntry.dataType() == DataTypes.ASCII_STRING || elEntry.dataType() == DataTypes.UINT) && elEntry.encodedData().length() > 0) {
            BasicPrimitiveConverter.writeAsciiString(elEntry.name(), outBuffer, error);
            BufferHelper.colon(outBuffer, error);
            converter.getPrimitiveHandler(elEntry.dataType()).encodeJson(decIter, outBuffer, error);
        } else {
            int type = elEntry.dataType();
            BasicPrimitiveConverter.writeAsciiString(elEntry.name(), outBuffer, error);
            BufferHelper.colon(outBuffer, error);
            BufferHelper.beginObject(outBuffer, error);
            BufferHelper.writeArrayAndColon(JSON_TYPE, outBuffer, false, error);
            BufferHelper.writeArray(converter.getDataType(elEntry.dataType()), outBuffer, true, error);
            BufferHelper.writeArrayAndColon(JSON_DATA, outBuffer, true, error);
            if (elEntry.dataType() < DataTypes.SET_PRIMITIVE_MAX) {
                converter.getPrimitiveHandler(elEntry.dataType()).encodeJson(decIter, outBuffer, error);
            } else if (elEntry.dataType() > DataTypes.NO_DATA) {
                converter.getContainerHandler(type).encodeJson(decIter, outBuffer, false, localSetDb, error);
            }
            BufferHelper.endObject(outBuffer, error);
        }
        return error.isSuccessful();
    }

    @Override
    void encodeRWF(JsonNode node, String stringKey, EncodeIterator iter, JsonConverterError error) {

        ElementList elementList = JsonFactory.createElementList();
        ElementEntry elementEntry = JsonFactory.createElementEntry();
        Buffer name = JsonFactory.createBuffer();

        if (node.getNodeType() != JsonNodeType.OBJECT) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected object type, found " + node.getNodeType().toString());
            return;
        }

        try {
            elementList.clear();
            elementList.applyHasStandardData();

            int ret = elementList.encodeInit(iter, null, 0);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding ElementList, code: " + ret);
                return;
            }

            Iterator<String> iterator =  node.fieldNames();
            while (iterator.hasNext()) {
                String next = iterator.next();
                JsonNode currEntryElem = node.get(next); //currEntryElem is an actual ElementEntry
                elementEntry.clear();
                name.clear();
                name.data(next);
                elementEntry.name(name);
                JsonNode data = null;
                if (currEntryElem.isTextual() || currEntryElem.isLong() || currEntryElem.isInt()) { //ASCII_STRING || UINT
                    elementEntry.dataType(currEntryElem.isTextual() ? DataTypes.ASCII_STRING : DataTypes.UINT);
                    data = currEntryElem;
                    if (error.isFailed())
                        return;
                } else if (currEntryElem.isObject()) { //Otherwise the type should be encoded as {"Type":"...", "Data" : ...}
                    JsonNode type = currEntryElem.path(JSON_TYPE);
                    data = currEntryElem.path(JSON_DATA);

                    if (type.isMissingNode()) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, "Missing key " + JSON_TYPE + " in ElementEntry", next);
                        return;
                    }
                    if (data.isMissingNode()) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, "Missing key " + JSON_DATA + " in ElementEntry", next);
                        return;
                    }

                    int dataType = converter.getDataType(type);
                    if (dataType == FAILURE) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_DATA_TYPE, "Unsupported data type: " + type.asText(), next);
                        return;
                    }
                    elementEntry.dataType(dataType);
                } else {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Unexpected token in ElementEntry, found " + currEntryElem.getNodeType() + " type", next);
                    return;
                }

                ret = elementEntry.encodeInit(iter, 0);
                if (ret < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding ElementEntry, code: " + ret, next);
                    return;
                }
                converter.decodeChunk(elementEntry.dataType(), data, next, iter, error);
                if (error.isFailed())
                    return;
                ret = elementEntry.encodeComplete(iter, true);
                if (ret < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding ElementEntry, code: " + ret, next);
                    return;

                }
            }
            ret = elementList.encodeComplete(iter, true);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding ElementEntry, code: " + ret, stringKey);
                return;
            }
            return;

        } finally {
            JsonFactory.releaseElementEntry(elementEntry);
            JsonFactory.releaseElementList(elementList);
            JsonFactory.releaseBuffer(name);
        }
    }

}
