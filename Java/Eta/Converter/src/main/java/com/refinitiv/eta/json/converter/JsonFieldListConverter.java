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

import java.util.Iterator;

class JsonFieldListConverter extends AbstractContainerTypeConverter {

    JsonFieldListConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.FIELD_LIST };
    }

    @Override
    Object getContainerObject()  { return JsonFactory.createFieldList(); }
    @Override
    Object getEntryObject() { return JsonFactory.createFieldEntry(); }

    @Override
    void releaseContainer(Object container) {JsonFactory.releaseFieldList((FieldList)container);}
    @Override
    void releaseEntry(Object entry) {JsonFactory.releaseFieldEntry((FieldEntry)entry);}

    @Override
    boolean hasEntries(Object container) {
        return (((FieldList) container).encodedEntries() != null && ((FieldList) container).encodedEntries().data() != null && ((FieldList) container).encodedEntries().length() != 0) ||
                (((FieldList) container).encodedSetData() != null && ((FieldList) container).encodedSetData().data() != null && ((FieldList) container).encodedSetData().length() != 0);
    }

    @Override
    protected int decodeEntry(DecodeIterator decIter, Object entryObj) {
        FieldEntry entry = (FieldEntry) entryObj;
        entry.clear();
        return entry.decode(decIter);
    }

    @Override
    protected int decodeContainer(DecodeIterator decIter, Object setDb, Object container) {
        FieldList list = (FieldList) container;
        list.clear();
        return list.decode(decIter, (LocalFieldSetDefDb) setDb);
    }

    @Override
    protected boolean writeContent(DecodeIterator decIter, JsonBuffer outBuffer, JsonConverterError error, Object container) {
        BufferHelper.beginObject(outBuffer, error);
        writeEntries(decIter, outBuffer, false, null, error, false, container);
        BufferHelper.endObject(outBuffer, error);
        return error.isSuccessful();
    }

    @Override
    protected boolean writeEntry(DecodeIterator decIter, JsonBuffer outBuffer, Object localSetDb, JsonConverterError error, Object entryObj, Object container) {

        FieldEntry fieldEntry = (FieldEntry) entryObj;
        DictionaryEntry def = converter.getDictionary().entry(fieldEntry.fieldId());
        if (def != null) {
            BasicPrimitiveConverter.writeAsciiString(def.acronym(), outBuffer, error);
            BufferHelper.colon(outBuffer, error);
            if (def.rwfType() < DataTypes.SET_PRIMITIVE_MAX) {
                if (def.rwfType() != DataTypes.ENUM || !converter.expandEnumFields()) {
                    converter.getPrimitiveHandler(def.rwfType()).encodeJson(decIter, outBuffer, error);
                } else {
                    ((JsonEnumerationConverter)converter.getPrimitiveHandler(DataTypes.ENUM)).encodeJson(decIter, def, outBuffer, error);
                }
            } else {
                converter.getContainerHandler(def.rwfType()).encodeJson(decIter, outBuffer, false, localSetDb, error);
            }
        }
        else
        {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_FID,
                    "encountered unexpected fid = " + fieldEntry.fieldId() + " while writing FieldEntry");
            return false;
        }

        return error.isSuccessful();
    }

    @Override
    void encodeRWF(JsonNode node, String stringKey, EncodeIterator iter, JsonConverterError error) {

        int ret = 0;

        if (!node.isObject()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Encountered unexpected type while decoding FieldList: " + node.getNodeType().toString(), stringKey);
            return;
        }

        FieldList fieldList = JsonFactory.createFieldList();
        FieldEntry fieldEntry = JsonFactory.createFieldEntry();

        try {
            fieldList.clear();
            fieldList.applyHasStandardData();

            ret = fieldList.encodeInit(iter, null, 0);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding FieldList, code: " + ret, stringKey);
                return;
            }

            if (converter.getDictionary() == null) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_DICT_NOT_INIT, "Dictionary not initialized");
                return;
            }

            for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
                fieldEntry.clear();
                String key = it.next();
                DictionaryEntry def;
                JsonNode currentNode = node.path(key);

                boolean isNumber = true;
                for (int i = 0; i < key.length(); i++) {
                    switch (key.charAt(i)) {
                        case '0': case '1': case '2':
                        case '3': case '4': case '5':
                        case '6': case '7': case '8':
                            break;
                        case '-':
                            if (i != 0)
                                isNumber = false;
                            break;
                        default:
                            isNumber = false;
                            break;
                    }

                    if (!isNumber)
                        break;
                }

                if (isNumber) { //int value of key
                    int num = Integer.valueOf(key);
                    fieldEntry.fieldId(num);
                    def = converter.getDictionary().entry(num);
                } else {
                    def = converter.getDictionary().entry(key);
                    if (def != null) {
                        fieldEntry.fieldId(def.fid());
                    }
                }
                if (def == null && converter.catchUnexpectedFids()) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_FID, "encountered unexpected fid = " + key + " while decoding FieldEntry");
                    return;
                }

                converter.dictionaryEntry(null);

                if (def != null) {
                    fieldEntry.dataType(def.rwfType());
                    converter.dictionaryEntry(def); //this dictionaryEntry is later used
                    ret = fieldEntry.encodeInit(iter, 0);
                    if (ret < CodecReturnCodes.SUCCESS) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding FieldEntry, code: " + ret, key);
                        return;
                    }
                    converter.decodeChunk(fieldEntry.dataType(), currentNode, key, iter, error);
                    if (error.isFailed())
                        return;
                    ret = fieldEntry.encodeComplete(iter, true);
                    if (ret < CodecReturnCodes.SUCCESS) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding FieldEntry, code: " + ret, key);
                        return;
                    }
                }
            }

            ret = fieldList.encodeComplete(iter, true);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding FieldList, code: " + ret, stringKey);
                return;
            }
            return;
        } finally {
            JsonFactory.releaseFieldEntry(fieldEntry);
            JsonFactory.releaseFieldList(fieldList);
        }

    }
}
