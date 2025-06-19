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

import java.util.Iterator;

import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_SERIES;
import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_SUMMARY;


class JsonSeriesConverter extends AbstractContainerTypeConverter {

    JsonSeriesConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.SERIES };
    }

    @Override
    Object getContainerObject()  { return JsonFactory.createSeries(); }
    @Override
    Object getEntryObject() { return JsonFactory.createSeriesEntry(); }

    @Override
    void releaseContainer(Object container) { JsonFactory.releaseSeries((Series)container); }
    @Override
    void releaseEntry(Object entry) { JsonFactory.releaseSeriesEntry((SeriesEntry)entry); }

    @Override
    boolean hasEntries(Object container) {
        return ((Series) container).encodedEntries() != null && ((Series) container).encodedEntries().data() != null && ((Series) container).encodedEntries().length() != 0;
    }

    @Override
    protected int decodeEntry(DecodeIterator decIter, Object entryObj) {
        SeriesEntry seriesEntry = (SeriesEntry) entryObj;
        seriesEntry.clear();
        return seriesEntry.decode(decIter);
    }

    @Override
    protected int decodeContainer(DecodeIterator decIter, Object setDb, Object container) {
        Series series = (Series) container;
        series.clear();
        return series.decode(decIter);
    }

    @Override
    protected boolean writeContent(DecodeIterator decIter, JsonBuffer outBuffer, JsonConverterError error, Object container) {

        BufferHelper.beginObject(outBuffer, error);
        Object localSetDb = null;
        boolean comma = false;
        Series series = (Series) container;

        try {
            if (series.checkHasSetDefs())
                localSetDb = getLocalSetDb(decIter, series.containerType(), error);

            if (series.checkHasSummaryData()) {
                writeSummaryData(decIter, series.containerType(), outBuffer, localSetDb, error);
                comma = true;
            }
            if (series.checkHasTotalCountHint()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_COUNTHINT, outBuffer, comma, error);
                BasicPrimitiveConverter.writeLong(series.totalCountHint(), outBuffer, error);
                comma = true;
            }
            if (series.encodedEntries().length() == 0)
                return BufferHelper.endObject(outBuffer, error);

            writeEntries(decIter, outBuffer, true, localSetDb, error, comma, container);
            BufferHelper.endObject(outBuffer, error);

            return true;
        } finally {
            if (localSetDb != null)
                returnLocalSetDb(series.containerType(), localSetDb);
        }

    }

    @Override
    protected boolean writeEntry(DecodeIterator decIter, JsonBuffer outBuffer, Object localSetDb, JsonConverterError error, Object entryObj, Object container) {

        int type = ((Series)container).containerType();
        BufferHelper.beginObject(outBuffer, error);
        converter.getContainerHandler(type).encodeJson(decIter, outBuffer, true, localSetDb, error);
        BufferHelper.endObject(outBuffer, error);
        return true;
    }

    @Override
    void encodeRWF(JsonNode node, String stringKey, EncodeIterator iter, JsonConverterError error) {

        Series series = JsonFactory.createSeries();
        SeriesEntry seriesEntry = JsonFactory.createSeriesEntry();
        int summaryType = DataTypes.UNKNOWN;
        int entryContainerType = DataTypes.UNKNOWN;
        JsonNode summary = null;
        JsonNode entryData = null;
        JsonNode entries = null;
        int ret = 0;

        series.clear();

        if (!node.isObject()) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected object type, found " + node.getNodeType().toString(), stringKey);
            return;
        }

        try {
            for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
                String key = it.next();
                JsonNode seriesNode = node.path(key);

                switch (key) {
                    case ConstCharArrays.JSON_ENTRIES:
                        if (!seriesNode.isArray()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected array type, found " + seriesNode.getNodeType(), JSON_SERIES + " " + key);
                            return;
                        }
                        entries = seriesNode;
                        break;
                    case ConstCharArrays.JSON_SUMMARY:
                        if (!seriesNode.isObject()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected object type, found " + seriesNode.getNodeType(), JSON_SERIES + " " + key);
                            return;
                        }
                        if (seriesNode.size() != 1) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, "Unexpected keys present", JSON_SERIES + " " + key);
                            return;
                        }
                        Iterator<String> curr = seriesNode.fieldNames();
                        String containerName = curr.next();
                        summaryType = converter.getContainerType(containerName);
                        if (summaryType == DataTypes.NO_DATA) {
                            if (converter.catchUnexpectedKeys()) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, "Unexpected key: " + containerName);
                                return;
                            } else {
                                continue;
                            }
                        }
                        summary = seriesNode.get(containerName);
                        break;
                    case ConstCharArrays.JSON_COUNTHINT:
                        if (!seriesNode.isInt()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected integer value, found " + seriesNode.getNodeType(), JSON_SERIES + " " + key);
                            return;
                        }
                        series.applyHasTotalCountHint();
                        series.totalCountHint(seriesNode.asInt());
                        break;
                    default:
                        if (converter.catchUnexpectedKeys()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, "Unexpected key: " + key, JSON_SERIES);
                            return;
                        }
                        break;
                }
            }

            if (summary != null) {
                encodeSeriesSummary(series, summary, summaryType, iter, error);
            }

            if (error.isFailed())
                return;

            if (entries != null) {
                for (int i = 0; i < entries.size(); i++) {
                    JsonNode seriesEntryNode = entries.get(i);
                    if (!seriesEntryNode.isObject()) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected object type, found " + seriesEntryNode.getNodeType().toString(), JSON_SERIES + ".entry[" + i + "]");
                        return;
                    }
                    seriesEntry.clear();
                    if (seriesEntryNode.isEmpty() || seriesEntryNode.isNull()) {
                        ret = seriesEntry.encode(iter);
                        if (ret != CodecReturnCodes.SUCCESS) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed to encode SeriesEntry entry[" + i + "]");
                            return;
                        } else {
                            continue;
                        }
                    } else {
                        String name = seriesEntryNode.fieldNames().next(); //must contain only one item
                        entryContainerType = converter.getContainerType(name);
                        if (entryContainerType == DataTypes.NO_DATA) {
                            if (converter.catchUnexpectedKeys()) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, "Unexpected key: " + name, JSON_SERIES + ".entry[" + i + "]");
                                return;
                            } else
                                continue;
                        }

                        if (summary != null) {
                            if (entryContainerType != summaryType) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed to encode Series: container types mismatch, found "
                                        + entryContainerType + " and " + summaryType, JSON_SERIES + ".entry[" + i + "]");
                                return;
                            }
                        } else if (series.containerType() == DataTypes.NO_DATA) {
                            series.containerType(entryContainerType);
                            ret = series.encodeInit(iter, 0, 0);
                            if (ret < CodecReturnCodes.SUCCESS) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Series, code: " + ret, JSON_SERIES);
                                return;
                            }
                        } else if (series.containerType() != entryContainerType) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Series: container types mismatch", JSON_SERIES + ".entry[" + i + "]");
                            return;
                        }

                        //process the actual container
                        entryData = seriesEntryNode.get(name);
                        if (entryData != null) {
                            ret = seriesEntry.encodeInit(iter, 0);
                            if (ret < CodecReturnCodes.SUCCESS) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding SeriesEntry, code: " + ret, JSON_SERIES + ".entry[" + i + "]");
                                return;
                            }
                            converter.getContainerHandler(series.containerType()).encodeRWF(entryData, JSON_SERIES + ".entry[" + i + "]", iter, error);
                            if (error.isFailed())
                                return;
                            ret = seriesEntry.encodeComplete(iter, true);
                            if (ret < CodecReturnCodes.SUCCESS) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Series entry, code: " + ret, JSON_SERIES + ".entry[" + i + "]");
                                return;
                            }
                        } else {
                            ret = seriesEntry.encode(iter);
                            if (ret < CodecReturnCodes.SUCCESS) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Series entry, code: " + ret, JSON_SERIES + ".entry[" + i + "]");
                                return;
                            }
                        }
                    }
                }
            }

            if (summary == null && (entries == null || entries.size() == 0)) {
                ret = series.encodeInit(iter, 0, 0);
                if (ret < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Series, code: " + ret, JSON_SERIES);
                    return;
                }
            }

            ret = series.encodeComplete(iter, true);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Series, code: " + ret, JSON_SERIES);
                return;
            }

            return;
        } catch (Exception ex) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Series, exception: " + ex.getMessage(), JSON_SERIES);
            return;
        } finally {
            JsonFactory.releaseSeries(series);
            JsonFactory.releaseSeriesEntry(seriesEntry);
        }
    }

    private void encodeSeriesSummary(Series series, JsonNode summary, int summaryType, EncodeIterator iter, JsonConverterError error) {
        series.applyHasSummaryData();
        series.containerType(summaryType);
        int ret = series.encodeInit(iter, 0, 0);
        if (ret < CodecReturnCodes.SUCCESS) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Series, code: " + ret, JSON_SERIES);
            return;
        }
        converter.getContainerHandler(summaryType).encodeRWF(summary,JSON_SUMMARY, iter, error);
        if (error.isFailed())
            return;
        ret = series.encodeSummaryDataComplete(iter, true);
        if (ret < CodecReturnCodes.SUCCESS) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Series, code: " + ret, JSON_SERIES);
            return;
        }
    }
}
