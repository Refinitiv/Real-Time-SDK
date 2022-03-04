/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

import java.util.HashMap;

abstract class AbstractContainerTypeConverter extends AbstractTypeConverter {

    private HashMap<Integer, String> containerNameMap = new HashMap<>();

    AbstractContainerTypeConverter(JsonAbstractConverter converter) {
        super(converter);
        containerNameMap.put(DataTypes.VECTOR, ConstCharArrays.JSON_VECTOR);
        containerNameMap.put(DataTypes.MAP, ConstCharArrays.JSON_MAP);
        containerNameMap.put(DataTypes.SERIES, ConstCharArrays.JSON_SERIES);
        containerNameMap.put(DataTypes.ELEMENT_LIST, ConstCharArrays.JSON_ELEMENTS);
        containerNameMap.put(DataTypes.FILTER_LIST, ConstCharArrays.JSON_FILTERLIST);
        containerNameMap.put(DataTypes.FIELD_LIST, ConstCharArrays.JSON_FIELDS);
        containerNameMap.put(DataTypes.OPAQUE, ConstCharArrays.JSON_OPAQUE);
        containerNameMap.put(DataTypes.XML, ConstCharArrays.JSON_XML);
        containerNameMap.put(DataTypes.JSON, ConstCharArrays.JSON_JSON);
        containerNameMap.put(DataTypes.ANSI_PAGE, ConstCharArrays.JSON_ANSI);
    }

    Object getContainerObject() { return null; }
    Object getEntryObject() { return null; }

    void releaseContainer(Object contianer) {}
    void releaseEntry(Object entry) {}

    boolean hasEntries(Object container) {return false;}

    protected abstract int decodeEntry(DecodeIterator decIter, Object entryObj);
    protected abstract boolean writeContent(DecodeIterator decIter, JsonBuffer outBuffer, JsonConverterError error, Object container);
    protected abstract int decodeContainer(DecodeIterator decIter, Object setDb, Object container);
    protected abstract boolean writeEntry(DecodeIterator decIterator, JsonBuffer outBuffer, Object localSetDb, JsonConverterError error, Object entryObj, Object container);

    protected boolean writeEntries(DecodeIterator decIter, JsonBuffer outBuffer, boolean writeTag, Object localSetDb, JsonConverterError error, boolean commaBefore, Object container) {

        boolean res = (!commaBefore || BufferHelper.comma(outBuffer, error)) && (!writeTag
                || (BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_ENTRIES, outBuffer, false, error) && BufferHelper.beginArray(outBuffer, error)));
        boolean comma = false;

        Object entry = getEntryObject();
        try {
            int ret = decodeEntry(decIter, entry);
            while (res && ret != CodecReturnCodes.END_OF_CONTAINER) {
                if (ret < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, null);
                    return false;
                } else {
                    if (comma)
                        res = res && BufferHelper.comma(outBuffer, error);
                    else
                        comma = true;

                    res = res && writeEntry(decIter, outBuffer, localSetDb, error, entry, container);
                }
                ret = decodeEntry(decIter, entry);
            }

            res = res && (!writeTag || BufferHelper.endArray(outBuffer, error));
            return res;
        } finally {
            releaseEntry(entry);
        }
    }
    
    protected Object getLocalSetDb(DecodeIterator decIter, int containerType, JsonConverterError error) {

        switch (containerType) {
            case DataTypes.FIELD_LIST:
                LocalFieldSetDefDb fieldSetDb = JsonFactory.createLocalFieldSetDefDb();
                if (fieldSetDb.decode(decIter) < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed decoding vector.");
                    return null;
                }
                return fieldSetDb;
            case DataTypes.ELEMENT_LIST:
                LocalElementSetDefDb elemSetDb = JsonFactory.createLocalElementSetDefDb();
                if (elemSetDb.decode(decIter) < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed decoding vector.");
                    return null;
                }
                return elemSetDb;
            default:
                break;
        }
        return null;
    }
    protected boolean returnLocalSetDb(int containerType, Object localSetDefDb) {

        switch (containerType) {
            case DataTypes.ELEMENT_LIST:
                JsonFactory.releaseElementSetDefDb((LocalElementSetDefDb) localSetDefDb);
                break;
            case DataTypes.FIELD_LIST:
                JsonFactory.releaseFieldSetDefDb((LocalFieldSetDefDb) localSetDefDb);
                break;
            default:
                break;
        }
        return true;
    }
    protected boolean writeSummaryData(DecodeIterator decIter, int containerType, JsonBuffer outBuffer, Object localSetDb, JsonConverterError error) {

        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_SUMMARY, outBuffer, false, error);
        BufferHelper.beginObject(outBuffer, error);
        converter.getContainerHandler(containerType).encodeJson(decIter, outBuffer, true, localSetDb, error);
        BufferHelper.endObject(outBuffer, error);

        return error.isSuccessful();
    }

    boolean encodeJson(DecodeIterator decIter, JsonBuffer outBuffer, boolean writeTag, Object setDb, JsonConverterError error) {

        Object container = getContainerObject();
        try {
            int ret = decodeContainer(decIter, setDb, container);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Failed decoding container " + getContainerName(dataTypes[0]) + ", code = " + ret);
                return false;
            }
            boolean res = (!writeTag || BufferHelper.writeArrayAndColon(getContainerName(dataTypes[0]), outBuffer, false, error));
            if (ret == CodecReturnCodes.BLANK_DATA || ret == CodecReturnCodes.NO_DATA)
                return res && BufferHelper.writeEmptyObject(outBuffer, error);

            res = res && writeContent(decIter, outBuffer, error, container);

            if (!res && error.isSuccessful())
                error.setError(JsonConverterErrorCodes.JSON_ERROR_OUT_OF_MEMORY, "Out of memory");
            return res;
        } finally {
            releaseContainer(container);
        }
    }

    protected String getContainerName(int containerType) {

        return containerNameMap.get(containerType);
    }
}
