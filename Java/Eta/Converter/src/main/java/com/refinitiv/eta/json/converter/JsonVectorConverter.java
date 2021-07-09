package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

import java.util.Iterator;

import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_VECTOR;
import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_SUMMARY;

class JsonVectorConverter extends AbstractContainerTypeConverter {

    JsonVectorConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.VECTOR };
    }

    @Override
    Object getContainerObject()  { return JsonFactory.createVector(); }
    @Override
    Object getEntryObject() { return JsonFactory.createVectorEntry(); }

    @Override
    void releaseContainer(Object container) {JsonFactory.releaseVector((Vector)container);}
    @Override
    void releaseEntry(Object entry) {JsonFactory.releaseVectorEntry((VectorEntry)entry);}

    @Override
    boolean hasEntries(Object container) {
        return ((Vector) container).encodedEntries() != null && ((Vector) container).encodedEntries().data() != null && ((Vector) container).encodedEntries().length() != 0;
    }

    @Override
    protected int decodeEntry(DecodeIterator decIter, Object entryObj) {
        VectorEntry entry = (VectorEntry) entryObj;
        entry.clear();
        return entry.decode(decIter);
    }

    @Override
    protected boolean writeContent(DecodeIterator decIter, JsonBuffer outBuffer, JsonConverterError error, Object container) {

        Object localSetDb = null;
        boolean comma = false;
        BufferHelper.beginObject(outBuffer, error);

        Vector vector = (Vector) container;

        if (vector.checkHasSetDefs())
            localSetDb = getLocalSetDb(decIter, vector.containerType(), error);

        if (vector.checkHasSummaryData() && vector.containerType() > DataTypes.NO_DATA) {
            if (!writeSummaryData(decIter, vector.containerType(), outBuffer, localSetDb, error))
                return false;
            comma = true;
        }

        if (vector.checkHasTotalCountHint()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_COUNTHINT, outBuffer, comma, error);
            BasicPrimitiveConverter.writeLong(vector.totalCountHint(), outBuffer, error);
            comma = true;
        }
        if (vector.checkSupportsSorting()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_SUPPORTSORTING, outBuffer, comma, error);
            BufferHelper.writeArray(ConstCharArrays.trueString, outBuffer, false, error);
            comma = true;
        }

        if (vector.encodedEntries().length() == 0)
            return BufferHelper.endObject(outBuffer, error);

        return writeEntries(decIter, outBuffer, true, localSetDb, error, comma, container) && BufferHelper.endObject(outBuffer, error);
    }

    @Override
    protected int decodeContainer(DecodeIterator decIter, Object setDb, Object container) {
        Vector vector = (Vector) container;
        vector.clear();
        return vector.decode(decIter);
    }

    @Override
    protected boolean writeEntry(DecodeIterator decIter, JsonBuffer outBuffer, Object localSetDb, JsonConverterError error, Object entryObj, Object container) {

        Vector vector = (Vector) container;
        VectorEntry vectorEntry = (VectorEntry) entryObj;

        BufferHelper.beginObject(outBuffer, error);
        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_INDEX, outBuffer, false, error);
        BasicPrimitiveConverter.writeLong(vectorEntry.index(), outBuffer, error);
        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_ACTION, outBuffer, true, error);
        BufferHelper.writeArray(getVectorEntryAction(vectorEntry.action()), outBuffer, true, error);

        if (vectorEntry.checkHasPermData()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PERMDATA, outBuffer, true, error);
            if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(vectorEntry.permData(), outBuffer, error))
                return false;
        }
        int containerType = vector.containerType();
        if (containerType > DataTypes.NO_DATA && vectorEntry.encodedData().length() > 0) {
            BufferHelper.comma(outBuffer, error);
            if (!converter.getContainerHandler(containerType).encodeJson(decIter, outBuffer, true, localSetDb, error))
                return false;
        }
        return BufferHelper.endObject(outBuffer, error);
    }


    @Override
    void encodeRWF(JsonNode node, String stringKey, EncodeIterator iter, JsonConverterError error) {

        Vector vector = JsonFactory.createVector();
        VectorEntry vectorEntry = JsonFactory.createVectorEntry();

        int ret = 0;
        JsonNode summary = null;
        JsonNode entries = null;
        JsonNode entryData = null;
        int summaryType = DataTypes.UNKNOWN;
        int containerType = DataTypes.UNKNOWN;

        vector.clear();

        if (!node.isObject()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected object type, found " + node.getNodeType().toString());
            return;
        }

        try {
            for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
                String key = it.next();
                JsonNode currentNode = node.path(key);
                switch (key) {
                    case JSON_SUMMARY:
                        if (!currentNode.isObject()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected object type, found " + currentNode.getNodeType().toString(), JSON_SUMMARY);
                            return;
                        }
                        if (currentNode.size() != 1) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Unexpected keys present", JSON_SUMMARY);
                            return;
                        }
                        Iterator<String> curr = currentNode.fieldNames(); //there should be only one value
                        String containerName = curr.next();
                        summaryType = converter.getContainerType(containerName);
                        if (summaryType == DataTypes.NO_DATA) {
                            if (converter.catchUnexpectedKeys()) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, "Unexpected key: " + containerName, JSON_SUMMARY);
                                return;
                            } else {
                                continue;
                            }
                        }
                        summary = currentNode.get(containerName);
                        break;
                    case ConstCharArrays.JSON_SUPPORTSORTING:
                        if (!currentNode.isBoolean()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected boolean value, found " + currentNode.getNodeType().toString(),
                                    ConstCharArrays.JSON_SUPPORTSORTING);
                            return;
                        }
                        if (currentNode.asBoolean())
                            vector.applySupportsSorting();
                        break;
                    case ConstCharArrays.JSON_COUNTHINT:
                        if (!currentNode.isInt()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected integer value, found " + currentNode.getNodeType().toString(),
                                    ConstCharArrays.JSON_COUNTHINT);
                            return;
                        }
                        vector.applyHasTotalCountHint();
                        vector.totalCountHint(currentNode.asInt());
                        break;
                    case ConstCharArrays.JSON_ENTRIES:
                        if (!currentNode.isArray()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected array type, found " + currentNode.getNodeType().toString(),
                                    ConstCharArrays.JSON_ENTRIES);
                            return;
                        }
                        entries = currentNode;
                        containerType = converter.getPayloadType(currentNode);
                        break;
                    default:
                        if (converter.catchUnexpectedKeys()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, "Unexpected key: " + key, stringKey);
                            return;
                        }
                        break;
                }
            }
            if (summary != null) {
                encodeVectorSummary(iter,vector, summary, summaryType, error);
                if (error.isFailed())
                    return;
            }

            boolean foundValidToken;

            if (entries != null) {
                for (int i = 0; i < entries.size(); i++) {
                    vectorEntry.clear();
                    JsonNode curr = entries.get(i);
                    if (!curr.isObject()) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected object type, found " + curr.getNodeType().toString());
                        return;
                    }
                    entryData = null;
                    for (Iterator<String> it = curr.fieldNames(); it.hasNext(); ) {
                        String name = it.next();
                        JsonNode child = curr.get(name);
                        foundValidToken = findValidVectorEntryToken(name, child, vectorEntry, error, i);

                        if (foundValidToken)
                            continue;
                        else if (error.isFailed())
                            return;

                        containerType = converter.getContainerType(name);
                        if (containerType == DataTypes.NO_DATA) {
                            if (converter.catchUnexpectedKeys()) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, "Encountered unexpected key in Vector Entry: " + name, "Vector entry[" + i + "]");
                                return;
                            } else
                                continue;
                        }

                        if (summary != null) {
                            if (containerType != DataTypes.NO_DATA && containerType != summaryType) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed to encode Vector: entry container type mismatch with Vector container type, " +
                                        "found containerType = " + summaryType + " for Summary and containerType = " + containerType + " for VectorEntry", "Vector entry[" + i + "]");
                                return;
                            }
                        } else if (vector.containerType() == DataTypes.NO_DATA) {
                            vector.containerType(containerType);
                            ret = vector.encodeInit(iter, 0, 0);
                            if (ret < CodecReturnCodes.SUCCESS) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Vector, code: " + ret, stringKey);
                                return;
                            }
                        } else if (vector.containerType() != containerType) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Vector: container types mismatch, found vector.containerType() = "
                                    + vector.containerType() + " and VectorEntry containerType = " + containerType, stringKey);
                            return;
                        }

                        entryData = child;
                    }
                    if (entryData != null) {
                        ret = vectorEntry.encodeInit(iter, 0);
                        if (ret < CodecReturnCodes.SUCCESS) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding VectorEntry, code: " + ret, "Vector entry[" + i + "]");
                            return;
                        }
                        converter.getContainerHandler(vector.containerType()).encodeRWF(entryData, converter.getDataType(vector.containerType()), iter, error);
                        if (error.isFailed())
                            return;
                        ret = vectorEntry.encodeComplete(iter, true);
                        if (ret < CodecReturnCodes.SUCCESS) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding VectorEntry, code: " + ret, "Vector entry[" + i + "]");
                            return;
                        }
                    } else {
                        if (summary == null && vector.containerType() == DataTypes.NO_DATA) {
                            vector.containerType(containerType);
                            ret = vector.encodeInit(iter, 0, 0);
                            if (ret < CodecReturnCodes.SUCCESS) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding VectorEntry, code: " + ret, "Vector entry[" + i + "]");
                                return;
                            }
                        }
                        ret = vectorEntry.encode(iter);
                        if (ret < CodecReturnCodes.SUCCESS) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding VectorEntry, code: " + ret, "Vector entry[" + i + "]");
                            return;
                        }
                    }
                }
            }

            if (summary == null && vector.containerType() == DataTypes.NO_DATA) {
                ret = vector.encodeInit(iter, 0, 0);
                if (ret < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Vector, code: " + ret, stringKey);
                    return;
                }
            }

            ret = vector.encodeComplete(iter, true);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Vector, code: " + ret, stringKey);
                return;
            }

            return;
        } catch (Exception ex) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Vector, exception: " + ex.getMessage(), JSON_VECTOR);
            return;
        } finally {
            JsonFactory.releaseVector(vector);
            JsonFactory.releaseVectorEntry(vectorEntry);
        }
    }

    private void encodeVectorSummary(EncodeIterator iter, Vector vector, JsonNode summary, int summaryType, JsonConverterError error) {

        vector.applyHasSummaryData();
        vector.containerType(summaryType);
        int ret = vector.encodeInit(iter, 0, 0);
        if (ret < CodecReturnCodes.SUCCESS) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Vector, code: " + ret);
            return;
        }

        converter.getContainerHandler(summaryType).encodeRWF(summary, JSON_SUMMARY, iter, error);
        if (error.isFailed())
            return;
        ret = vector.encodeSummaryDataComplete(iter, true);
        if (ret < CodecReturnCodes.SUCCESS) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Vector, code: " + ret, JSON_SUMMARY);
            return;
        }
    }

    private boolean findValidVectorEntryToken(String name, JsonNode child, VectorEntry vectorEntry, JsonConverterError error, int position) {
        switch (name) {
            case ConstCharArrays.JSON_ACTION:
                if (!child.isTextual()) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected string type for Vector entry action, found " + child.getNodeType().toString(), "Vector entry[" + position + "]." + name);
                    return false;
                }
                int action = getVectorAction(child.asText());
                if (action == -1) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Unexpected parameter for for Vector entry action: " + child.get(name).asText(), "Vector entry[" + position + "]." +  name);
                    return false;
                }
                vectorEntry.action(action);
                return true;
            case ConstCharArrays.JSON_INDEX:
                if (!child.isInt() && !child.isLong()) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected long type, found " + child.getNodeType().toString(), "Vector entry[" + position + "]." + name);
                    return false;
                }
                vectorEntry.index(child.asLong());
                return true;
            case ConstCharArrays.JSON_PERMDATA:
                if (!child.isTextual()) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected string type, found " + child.getNodeType().toString(), "Vector entry[" + position + "]." + name);
                    return false;
                }
                vectorEntry.applyHasPermData();
                converter.getPrimitiveHandler(DataTypes.BUFFER).decodeJson(child, vectorEntry.permData(), error);
                if (error.isFailed())
                    return false;
                return true;
            default:
                return false;
        }
    }

    private String getVectorEntryAction(int entryAction) {
        if (entryAction >= VectorEntryActions.UPDATE && entryAction <= VectorEntryActions.DELETE) {
            return ConstCharArrays.entryActionStrings[entryAction - 1];
        }
        return null;
    }

    private int getVectorAction(String action) {
        switch (action) {
            case ConstCharArrays.ACTION_STR_UPDATE:
                return VectorEntryActions.UPDATE;
            case ConstCharArrays.ACTION_STR_SET:
                return VectorEntryActions.SET;
            case ConstCharArrays.ACTION_STR_INSERT:
                return VectorEntryActions.INSERT;
            case ConstCharArrays.ACTION_STR_DELETE:
                return VectorEntryActions.DELETE;
            case ConstCharArrays.ACTION_STR_CLEAR:
                return VectorEntryActions.CLEAR;
            default:
                return -1;
        }
    }
}
