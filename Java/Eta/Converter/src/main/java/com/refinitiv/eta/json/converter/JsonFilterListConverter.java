package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

import java.util.Iterator;

class JsonFilterListConverter extends AbstractContainerTypeConverter {

    JsonFilterListConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.FILTER_LIST };
    }

    @Override
    Object getContainerObject()  { return JsonFactory.createFilterList(); }
    @Override
    Object getEntryObject() { return JsonFactory.createFilterEntry(); }

    @Override
    void releaseContainer(Object container) { JsonFactory.releaseFilterList((FilterList)container); }
    @Override
    void releaseEntry(Object entry) { JsonFactory.releaseFilterEntry((FilterEntry)entry); }

    @Override
    boolean hasEntries(Object container) {
        return ((FilterList) container).encodedEntries() != null && ((FilterList) container).encodedEntries().data() != null && ((FilterList) container).encodedEntries().length() != 0;
    }

    @Override
    void encodeRWF(JsonNode node, String stringKey, EncodeIterator iter, JsonConverterError error) {

        JsonNode entries = null;
        JsonNode entry = null;
        JsonNode entryData = null;

        boolean foundValidToken = false;
        int entryType = DataTypes.UNKNOWN;
        String key = null;

        int ret = 0;

        if (!node.isObject()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected object type, found " + node.getNodeType().toString(), stringKey);
            return;
        }

        FilterList filterList = JsonFactory.createFilterList();
        FilterEntry filterEntry = JsonFactory.createFilterEntry();

        try {
            filterList.clear();
            filterList.containerType(DataTypes.ELEMENT_LIST); //default value
            Iterator<String> iterator =  node.fieldNames();
            while (iterator.hasNext()) { //Process elements of the FilterList
                String next = iterator.next();
                JsonNode curr = node.get(next);
                switch (next) {
                    case ConstCharArrays.JSON_ENTRIES:
                        if (!curr.isArray()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected array type, found type " + curr.getNodeType().toString(), next);
                            return;
                        }
                        entries = curr;
                        break;
                    case ConstCharArrays.JSON_COUNTHINT:
                        if (!curr.isInt()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected integer type, found type " + curr.getNodeType().toString(), next);
                            return;
                        }
                        filterList.applyHasTotalCountHint();
                        filterList.totalCountHint(curr.asInt());
                        break;
                    default:
                        if (converter.catchUnexpectedKeys()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, "Unexpected key: " + next);
                            return;
                        }
                        break;
                }
            } //end while

            ret = filterList.encodeInit(iter);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding FilterEntry code: " + ret, stringKey);
                return;
            }

            if (entries != null) {
                for (int i = 0; i < entries.size(); i++) { //process entries
                    entry = entries.get(i);
                    if (!entry.isObject()) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected object type, found " + entry.getNodeType().toString(), ConstCharArrays.JSON_ENTRIES);
                        return;
                    }
                    filterEntry.clear();
                    entryData = null;
                    filterEntry.applyHasContainerType();
                    filterEntry.containerType(filterList.containerType());
                    Iterator<String> entryIter = entry.fieldNames();
                    boolean hasAction = false;
                    boolean hasId = false;
                    while (entryIter.hasNext()) { //Process the internals of the current entry: ID, ACTION, PERMDATA and find Data token
                        key = entryIter.next();
                        JsonNode entryToken = entry.get(key);
                        foundValidToken = false;
                        switch (key) {
                            case ConstCharArrays.JSON_ID:
                                foundValidToken = true;
                                hasId = true;
                                if (!entryToken.isInt()) {
                                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected integer type, found type " + entryToken.getNodeType().toString(),
                                            "entry[" + i + "]." + key);
                                    return;
                                }
                                filterEntry.id(entryToken.asInt());
                                break;
                            case ConstCharArrays.JSON_ACTION:
                                foundValidToken = true;
                                hasAction = true;
                                if (!(entryToken.isInt() || entryToken.isTextual())) {
                                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Unexpected Action type: " + entryToken.getNodeType().toString(),
                                            "entry[" + i + "]." + key);
                                    return;
                                }
                                if (entryToken.isTextual()) {
                                    int action = getAction(entryToken.asText());
                                    if (action == 0) {
                                        error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Unknown FilterEntry action: " + entryToken.asText(),
                                                "entry[" + i + "]." + key);
                                        return;
                                    } else
                                        filterEntry.action(action);
                                } else if (entryToken.isInt() && entryToken.asInt() >= FilterEntryActions.UPDATE && entryToken.asInt() <= FilterEntryActions.CLEAR) {
                                    filterEntry.action(entryToken.asInt());
                                } else {
                                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Unknown FilterList entry action: " + entryToken.asText(),
                                            "entry[" + i + "]." + key);
                                    return;
                                }
                                break;
                            case ConstCharArrays.JSON_PERMDATA:
                                foundValidToken = true;
                                if (!entryToken.isTextual()) {
                                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected string value for Permission Data, found " + entryToken.getNodeType().toString(),
                                            "entry[" + i + "]." + key);
                                    return;
                                }
                                filterEntry.applyHasPermData();
                                converter.getPrimitiveHandler(DataTypes.BUFFER).decodeJson(entryToken, filterEntry.permData(), error);
                                if (error.isFailed())
                                    return;
                                break;
                            default:
                                break;
                        }

                        if (foundValidToken)
                            continue;

                        entryType = converter.getContainerType(key);
                        if (entryType == DataTypes.NO_DATA) {
                            if (converter.catchUnexpectedKeys()) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, "Unexpected key: " + key, "entry[" + i + "]");
                                return;
                            } else
                                continue;
                        }
                        filterEntry.applyHasContainerType();
                        filterEntry.containerType(entryType);
                        entryData  = entryToken;
                    } // end While for entry

                    if (!hasAction) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, "Missing entry Action", "entry[" + i + "]");
                        return;
                    }
                    if (!hasId) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, "Missing entry Id", "entry[" + i + "]");
                        return;
                    }
                    //encode FilterEntry
                    if (entryData != null && filterEntry.action() != FilterEntryActions.CLEAR) {
                        ret = filterEntry.encodeInit(iter, 0);
                        if (ret < CodecReturnCodes.SUCCESS) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding FilterList entry, code: " + ret, "entry[" + i + "]");
                            return;
                        }
                        converter.getContainerHandler(entryType).encodeRWF(entryData, key, iter, error);
                        if (error.isFailed())
                            return;
                        ret = filterEntry.encodeComplete(iter, true);
                        if (ret < CodecReturnCodes.SUCCESS) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding FilterList entry, code: " + ret, "entry[" + i + "]");
                            return;
                        }
                    } else {
                        ret = filterEntry.encode(iter);
                        if (ret < CodecReturnCodes.SUCCESS) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding FilterList entry, code: " + ret, "entry[" + i + "]");
                            return;
                        }
                    }
                } //end processing entries
            } //end if

            ret = filterList.encodeComplete(iter, true);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR, "Encoding failed, code = " + ret, stringKey);
                return;
            }

            return;
        } finally {
            JsonFactory.releaseFilterEntry(filterEntry);
            JsonFactory.releaseFilterList(filterList);
        }
    }

    @Override
    protected int decodeEntry(DecodeIterator decIter, Object entryObj) {
        FilterEntry entry = (FilterEntry) entryObj;
        entry.clear();
        return entry.decode(decIter);
    }

    @Override
    protected boolean writeContent(DecodeIterator decIter, JsonBuffer outBuffer, JsonConverterError error, Object container) {

        boolean comma = false;
        BufferHelper.beginObject(outBuffer, error);
        FilterList filterList = (FilterList) container;

        if (filterList.checkHasTotalCountHint()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_COUNTHINT, outBuffer, false, error);
            BasicPrimitiveConverter.writeLong(filterList.totalCountHint(), outBuffer, error);
            comma = true;
        }

        if (filterList.encodedEntries().length() == 0)
            return BufferHelper.endObject(outBuffer, error);

        return writeEntries(decIter, outBuffer, true, null, error, comma, container) && BufferHelper.endObject(outBuffer, error);
    }

    @Override
    protected int decodeContainer(DecodeIterator decIter, Object setDb, Object container) {
        FilterList list = (FilterList) container;
        list.clear();
        return list.decode(decIter);
    }

    @Override
    protected boolean writeEntry(DecodeIterator decIter, JsonBuffer outBuffer, Object localSetDb, JsonConverterError error, Object entryObj, Object container) {

        FilterList filterList = (FilterList) container;
        FilterEntry filterEntry = (FilterEntry) entryObj;
        int containerType = filterEntry.checkHasContainerType() ? filterEntry.containerType() : filterList.containerType();

        if (filterEntry.action() == 0) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR, "FilterList Entry action is 0");
            return false;
        }

        BufferHelper.beginObject(outBuffer, error);
        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_ID, outBuffer, false, error);
        BasicPrimitiveConverter.writeLong(filterEntry.id(), outBuffer, error);
        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_ACTION, outBuffer, true, error);
        BufferHelper.writeArray(getAction(filterEntry.action()), outBuffer, true, error);

        if (filterEntry.checkHasPermData()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PERMDATA, outBuffer, true, error);
            converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(filterEntry.permData(), outBuffer, error);
        }

        if (filterEntry.encodedData().length() > 0 && containerType > DataTypes.NO_DATA) {
            BufferHelper.comma(outBuffer, error);
            converter.getContainerHandler(containerType).encodeJson(decIter, outBuffer, true, localSetDb, error);
        }

        return BufferHelper.endObject(outBuffer, error);
    }


    private String getAction(int action) {
        if (action >= FilterEntryActions.UPDATE && action <= FilterEntryActions.CLEAR)
            return ConstCharArrays.entryActionStrings[action - 1];
        return null;
    }

    private int getAction(String key) {
        switch (key) {
            case ConstCharArrays.ACTION_STR_UPDATE:
                return FilterEntryActions.UPDATE;
            case ConstCharArrays.ACTION_STR_SET:
                return FilterEntryActions.SET;
            case ConstCharArrays.ACTION_STR_CLEAR:
                return FilterEntryActions.CLEAR;
            default:
                return 0;
        }
    }
}
