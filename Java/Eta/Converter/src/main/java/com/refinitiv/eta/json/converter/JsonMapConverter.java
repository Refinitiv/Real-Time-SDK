/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.Double;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.Float;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

import java.nio.ByteBuffer;
import java.util.Iterator;
import java.util.Objects;

import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_MAP;

class JsonMapConverter extends AbstractContainerTypeConverter {

    JsonMapConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.MAP };
    }

    @Override
    Object getContainerObject()  { return JsonFactory.createMap(); }
    @Override
    Object getEntryObject() { return JsonFactory.createMapEntry(); }

    @Override
    void releaseContainer(Object container) {JsonFactory.releaseMap((Map)container);}
    @Override
    void releaseEntry(Object entry) {JsonFactory.releaseMapEntry((MapEntry)entry);}

    @Override
    boolean hasEntries(Object container) {
        return ((Map) container).encodedEntries().data() != null && ((Map) container).encodedEntries() != null && ((Map) container).encodedEntries().length() != 0;
    }

    @Override
    protected int decodeEntry(DecodeIterator decIter, Object entryObj) {
        MapEntry mapEntry = (MapEntry) entryObj;
        mapEntry.clear();
        return mapEntry.decode(decIter, null);
    }

    @Override
    protected boolean writeContent(DecodeIterator decIter, JsonBuffer outBuffer, JsonConverterError error, Object container) {

        Object localSetDb = null;
        Map map = (Map) container;

        try {
            if (map.checkHasSetDefs())
                localSetDb = getLocalSetDb(decIter, map.containerType(), error);

            boolean res = true;
            BufferHelper.beginObject(outBuffer, error);
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEYTYPE, outBuffer, false, error);
            BufferHelper.writeArray(converter.getDataType(map.keyPrimitiveType()), outBuffer, true, error);

            if (map.checkHasSummaryData() && map.containerType() > DataTypes.NO_DATA) {
                BufferHelper.comma(outBuffer, error);
                if (!writeSummaryData(decIter, map.containerType(), outBuffer, localSetDb, error))
                    return false;
            }

            if (map.checkHasTotalCountHint()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_COUNTHINT, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(map.totalCountHint(), outBuffer, error);
            }

            if (map.checkHasKeyFieldId()) {
                BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEYFIELDID, outBuffer, true, error);
                BasicPrimitiveConverter.writeLong(map.keyFieldId(), outBuffer, error);
            }

            if (map.encodedEntries().length() == 0) {
                return BufferHelper.endObject(outBuffer, error);
            }

            if (!writeEntries(decIter, outBuffer, true, localSetDb, error, true, container))
                return false;

            BufferHelper.endObject(outBuffer, error);

            return res;

        } finally {
            if (localSetDb != null)
                returnLocalSetDb(map.containerType(), localSetDb);
        }
    }

    @Override
    protected int decodeContainer(DecodeIterator decIter, Object setDb, Object container) {
        Map map = (Map) container;
        map.clear();
        return map.decode(decIter);
    }

    @Override
    protected boolean writeEntry(DecodeIterator decIter, JsonBuffer outBuffer, Object localSetDb, JsonConverterError error, Object entryObj, Object container) {

        Map map = (Map) container;
        MapEntry mapEntry = (MapEntry) entryObj;
        boolean res = BufferHelper.beginObject(outBuffer, error);
        String action = getMapEntryAction(mapEntry.action());

        if (action == null)
            return false;

        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_ACTION, outBuffer, false, error);
        BufferHelper.writeArray(action, outBuffer, true, error);

        BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_KEY, outBuffer, true, error);
        if (!converter.getPrimitiveHandler(map.keyPrimitiveType()).encodeJson(mapEntry.encodedKey(), outBuffer, error))
            return false;

        
        if (mapEntry.checkHasPermData()) {
            BufferHelper.writeArrayAndColon(ConstCharArrays.JSON_PERMDATA, outBuffer, true, error);
            if (!converter.getPrimitiveHandler(DataTypes.BUFFER).encodeJson(mapEntry.permData(), outBuffer, error))
                return false;
        }

        if (mapEntry.encodedData().length() == 0)
            return res && BufferHelper.endObject(outBuffer, error);

        int type =((Map) container).containerType();
        BufferHelper.comma(outBuffer, error);
        if (!converter.getContainerHandler(type).encodeJson(decIter, outBuffer, true, localSetDb, error))
            return false;
        BufferHelper.endObject(outBuffer, error);

        return res;
    }

    @Override
    protected void encodeRWF(JsonNode node, String keyString, EncodeIterator iter, JsonConverterError error) {

        Map map = JsonFactory.createMap();
        map.clear();
        MapEntry mapEntry = JsonFactory.createMapEntry();
        Buffer buf = JsonFactory.createBuffer();
        buf.data(ByteBuffer.allocate(0));

        int ret = 0;

        try {
            JsonNode summary = null;
            JsonNode entries = null;
            JsonNode entryData = null;
            int summaryType = DataTypes.UNKNOWN;
            int entryContainerType = DataTypes.NO_DATA;
            Object encKey = null;
            boolean hasKeyType = false;

            map.clear();

            if (!node.isObject()) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE,
                        "Expected object type, found " + node.getNodeType().toString(), keyString);
                return;
            }

            for (Iterator<String> it = node.fieldNames(); it.hasNext(); ) {
                String key = it.next();
                JsonNode currentNode = node.path(key);
                switch (key) {
                    case ConstCharArrays.JSON_SUMMARY:
                        if (!currentNode.isObject()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE,
                                    "Expected object type for Summary, found " + currentNode.getNodeType().toString(), ConstCharArrays.JSON_SUMMARY);
                            return;
                        }
                        if (currentNode.size() != 1) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY,
                                    "Unexpected keys present", ConstCharArrays.JSON_SUMMARY);
                            return;
                        }
                        Iterator<String> curr = currentNode.fieldNames();
                        String containerName = curr.next();
                        summaryType = converter.getContainerType(containerName);
                        if (summaryType == DataTypes.NO_DATA) {
                            if (converter.catchUnexpectedKeys()) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY,
                                        "Unexpected key: " + containerName, ConstCharArrays.JSON_SUMMARY);
                                return;
                            } else {
                                continue;
                            }
                        }
                        summary = currentNode.get(containerName);
                        break;
                    case ConstCharArrays.JSON_COUNTHINT:
                        if (!currentNode.isInt()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE,
                                    "Expected integer type, found " + currentNode.getNodeType().toString(),
                                    ConstCharArrays.JSON_COUNTHINT);
                            return;
                        }
                        map.applyHasTotalCountHint();
                        map.totalCountHint(currentNode.asInt());
                        break;
                    case ConstCharArrays.JSON_KEYTYPE:
                        int type = converter.getDataType(currentNode);
                        if (type == CodecReturnCodes.FAILURE) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_DATA_TYPE,
                                    "Unexpected type: " + type, ConstCharArrays.JSON_KEYTYPE);
                            return;
                        } else if (type >= DataTypes.SET_PRIMITIVE_MAX) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_INVALID_PRIMITIVE_TYPE,
                                    "Unexpected primitive type: " + type, ConstCharArrays.JSON_KEYTYPE);
                            return;
                        }
                        hasKeyType = true;
                        map.keyPrimitiveType(type);
                        break;
                    case ConstCharArrays.JSON_KEYFIELDID:
                        if (!currentNode.isInt()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE,
                                    "Expected integer value, found " + currentNode.getNodeType().toString(), ConstCharArrays.JSON_KEYFIELDID);
                            return;
                        }
                        map.applyHasKeyFieldId();
                        map.keyFieldId(currentNode.asInt());
                        break;
                    case ConstCharArrays.JSON_ENTRIES:
                        if (!currentNode.isArray()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE,
                                    "Expected array type, found type " + currentNode.getNodeType().toString(), ConstCharArrays.JSON_ENTRIES);
                            return;
                        }
                        entries = currentNode;
                        entryContainerType = converter.getPayloadType(currentNode);
                        break;
                    default:
                        if (converter.catchUnexpectedKeys()) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, "Unexpected key: " + key, keyString);
                            return;
                        }
                        break;
                }
            }

            if (!hasKeyType) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, "Missing key type", keyString);
                return;
            }

            if (summaryType != DataTypes.UNKNOWN) {
                map.applyHasSummaryData();
                map.containerType(summaryType);
            } else if (entryContainerType != DataTypes.UNKNOWN) {
                map.containerType(entryContainerType);
            } else {
                map.containerType(DataTypes.NO_DATA);
            }

            ret = map.encodeInit(iter, 0, 0);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Map, code: " + ret);
                return;
            }

            if (Objects.nonNull(summary)) {
                converter.getContainerHandler(summaryType).encodeRWF(summary, null, iter, error);
                if (error.isFailed())
                    return;
                ret = map.encodeSummaryDataComplete(iter, true);
                if (ret < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Map, code: " + ret);
                    return;
                }
            }

            boolean foundValidToken;
            if (entries != null) {
                for (int i = 0; i < entries.size(); i++) {
                    mapEntry.clear();
                    entryData = null;
                    encKey = null;
                    JsonNode curr = entries.get(i);
                    boolean hasEntriesKey = false;
                    boolean hasEntriesAction = false;

                    if (!curr.isObject()) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected object type for Map entry[" + i + "], found " + curr.getNodeType().toString());
                        return;
                    }
                    for (Iterator<String> it = curr.fieldNames(); it.hasNext(); ) {
                        String name = it.next();
                        foundValidToken = false;
                        JsonNode child = curr.get(name);
                        switch (name) {
                            case ConstCharArrays.JSON_ACTION:
                                foundValidToken = true;
                                hasEntriesAction = true;
                                if (!child.isTextual()) {
                                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected string type for Map entry[" + i + "] action, found " + child.getNodeType().toString());
                                    return;
                                }
                                setMapEntryAction(child, mapEntry, error, i);
                                if (error.isFailed())
                                    return;
                                break;
                            case ConstCharArrays.JSON_KEY:
                                hasEntriesKey = true;
                                foundValidToken = true;
                                encKey = converter.getPrimitiveHandler(map.keyPrimitiveType()).getPrimitiveType();
                                converter.getPrimitiveHandler(map.keyPrimitiveType()).decodeJson(child, encKey, error);
                                if (error.isFailed())
                                    return;
                                if (encKey == null)
                                    encKey = buf;
                                break;
                            case ConstCharArrays.JSON_PERMDATA:
                                foundValidToken = true;
                                if (!child.isTextual()) {
                                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Expected string type for Permission Data in Map entry[" + i + "], found: " + child.getNodeType().toString());
                                    return;
                                }
                                converter.getPrimitiveHandler(DataTypes.BUFFER).decodeJson(child, mapEntry.permData(), error);
                                if (error.isFailed())
                                    return;
                                mapEntry.applyHasPermData();
                                break;
                            default:
                                break;
                        }

                        if (foundValidToken)
                            continue;

                        entryContainerType = converter.getContainerType(name);

                        if (entryContainerType == DataTypes.NO_DATA) {
                            if (converter.catchUnexpectedKeys()) {
                                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_KEY, "Unexpected key in Map entry[" + i + "]: " + name);
                                return;
                            } else
                                continue;
                        }
                        entryData = child;
                    }

                    if (summary != null) { //set the check of NO_DATA because otherwise if we have only DELETE entries, we'll get error here
                        if (entryContainerType != DataTypes.NO_DATA && entryContainerType != summaryType) { //if there is summary, we already know the container type
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed to encode Map: container types mismatch, found summary container type = " + summaryType + ", entry container type = " + entryContainerType);
                            return;
                        }
                    } else if (map.containerType() != entryContainerType) { //some entry has already been encoded previously, check that types coincide
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding MapEntry: entry[" + i + "] container type mismatch with Map container type, found " + map.containerType() + " and " + entryContainerType + " types.");
                        return;
                    }

                    if (!hasEntriesKey) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, "Missing entry key", "Map entry[" + i + "]");
                        return;
                    }
                    if (!hasEntriesAction) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, "Missing entry action", "Map entry[" + i + "]");
                        return;
                    }

                    if (encKey == null) {
                        error.setError(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, "Missing entry key", "Map entry[" + i + "]");
                        return;
                    }

                    if (entryData != null && mapEntry.action() != MapEntryActions.DELETE) {
                        ret = encodeMapEntryInit(mapEntry, iter, encKey);
                        if (ret < CodecReturnCodes.SUCCESS) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding MapEntry, code: " + ret, "Map entry[" + i + "]");
                            return;
                        }
                        converter.getContainerHandler(map.containerType()).encodeRWF(entryData, "Map entry[" + i + "]", iter, error);
                        if (error.isFailed())
                            return;
                        ret = mapEntry.encodeComplete(iter, true);
                        if (ret < CodecReturnCodes.SUCCESS) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding MapEntry, code: " + ret, "Map entry[" + i + "]");
                            return;
                        }
                    } else {
                        ret = encodeMapEntry(mapEntry, iter, encKey);
                        if (ret < CodecReturnCodes.SUCCESS) {
                            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding MapEntry, code: " + ret, "Map entry[" + i + "]");
                            return;
                        }
                    }

                    if (encKey != null) {
                        converter.getPrimitiveHandler(map.keyPrimitiveType()).releasePrimitiveType(encKey);
                    }
                }
            }

            if (summary == null && map.containerType() == DataTypes.UNKNOWN) {
                map.containerType(DataTypes.NO_DATA);
                ret = map.encodeInit(iter, 0, 0);
                if (ret < CodecReturnCodes.SUCCESS) {
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Map, code: " + ret, keyString);
                    return;
                }
            }

            ret = map.encodeComplete(iter,true);
            if (ret < CodecReturnCodes.SUCCESS) {
                error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Map, code: " + ret, keyString);
                return;
            }

            return;

        } catch (Exception ex) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_RSSL_ENCODE_ERROR, "Failed encoding Map, exception: " + ex.getMessage(), JSON_MAP);
            return;
        }finally {
            JsonFactory.releaseMap(map);
            JsonFactory.releaseMapEntry(mapEntry);
            JsonFactory.releaseBuffer(buf);
        }
    }

    private String getMapEntryAction(int action) {
        if (action >= MapEntryActions.UPDATE && action <= MapEntryActions.DELETE)
            return ConstCharArrays.mapEntryActionStrings[action - 1];
        return null;
    }

    private void setMapEntryAction(JsonNode child, MapEntry mapEntry, JsonConverterError error, int position) {
        switch (child.asText()) {
            case ConstCharArrays.ACTION_STR_ADD:
                mapEntry.action(MapEntryActions.ADD);
                break;
            case ConstCharArrays.ACTION_STR_UPDATE:
                mapEntry.action(MapEntryActions.UPDATE);
                break;
            case ConstCharArrays.ACTION_STR_DELETE:
                mapEntry.action(MapEntryActions.DELETE);
                break;
            default:
                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Unexpected value for Map Entry action: " + child.asText(), "entry[" + position + "]");
                return;
        }
    }

    private static int encodeMapEntryInit(MapEntry mapEntry, EncodeIterator encIter, Object key) {
        if (key instanceof Int)
            return mapEntry.encodeInit(encIter, (Int) key, 0);
        if (key instanceof UInt)
            return mapEntry.encodeInit(encIter, (UInt) key, 0);
        if (key instanceof Real)
            return mapEntry.encodeInit(encIter, (Real) key, 0);
        if (key instanceof Float)
            return mapEntry.encodeInit(encIter, (Float) key, 0);
        if (key instanceof Double)
            return mapEntry.encodeInit(encIter, (Double) key, 0);
        if (key instanceof Buffer)
            return mapEntry.encodeInit(encIter, (Buffer) key, 0);
        if (key instanceof Enum)
            return mapEntry.encodeInit(encIter, (Enum) key, 0);
        if (key instanceof Date)
            return mapEntry.encodeInit(encIter, (Date) key, 0);
        if (key instanceof DateTime)
            return mapEntry.encodeInit(encIter, (DateTime) key, 0);
        if (key instanceof Time)
            return mapEntry.encodeInit(encIter, (Time) key, 0);
        if (key instanceof Qos)
            return mapEntry.encodeInit(encIter, (Qos) key, 0);
        if (key instanceof State)
            return mapEntry.encodeInit(encIter, (State) key, 0);

        return CodecReturnCodes.FAILURE;
    }

    private static int encodeMapEntry(MapEntry mapEntry, EncodeIterator encIter, Object key) {
        if (key instanceof Int)
            return mapEntry.encode(encIter, (Int) key);
        if (key instanceof UInt)
            return mapEntry.encode(encIter, (UInt) key);
        if (key instanceof Real)
            return mapEntry.encode(encIter, (Real) key);
        if (key instanceof Float)
            return mapEntry.encode(encIter, (Float) key);
        if (key instanceof Double)
            return mapEntry.encode(encIter, (Double) key);
        if (key instanceof Buffer)
            return mapEntry.encode(encIter, (Buffer) key);
        if (key instanceof Enum)
            return mapEntry.encode(encIter, (Enum) key);
        if (key instanceof Date)
            return mapEntry.encode(encIter, (Date) key);
        if (key instanceof DateTime)
            return mapEntry.encode(encIter, (DateTime) key);
        if (key instanceof Time)
            return mapEntry.encode(encIter, (Time) key);
        if (key instanceof Qos)
            return mapEntry.encode(encIter, (Qos) key);
        if (key instanceof State)
            return mapEntry.encode(encIter, (State) key);

        return CodecReturnCodes.FAILURE;
    }
}
