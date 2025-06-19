/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.DataDictionary;

import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

class JsonConverterBuilderImpl implements JsonConverterBuilder {
    private Map<Integer, Object> propertiesMap = new HashMap<>();
    private ServiceNameIdConverter serviceNameIdConverter;
    private DataDictionary dataDictionary;

    @Override
    public JsonConverterBuilder setProperties(Map<Integer, Object> properties) {
        propertiesMap.putAll(properties);
        return this;
    }

    @Override
    public JsonConverterBuilder setProperty(int propertyId, boolean enabled) {
        propertiesMap.put(propertyId, enabled);
        return this;
    }

    @Override
    public JsonConverterBuilder setProperty(int propertyId, int intValue) {
        propertiesMap.put(propertyId, intValue);
        return this;
    }

    @Override
    public JsonConverterBuilder setServiceConverter(ServiceNameIdConverter serviceNameIdConverter) {
        this.serviceNameIdConverter = serviceNameIdConverter;
        return this;
    }

    @Override
    public JsonConverterBuilder setDictionary(DataDictionary dataDictionary) {
        this.dataDictionary = dataDictionary;
        return this;
    }

    @Override
    public JsonConverter build(JsonConverterError error) {
        switch (getProtocolVer()) {
            case JsonProtocol.JSON_JPT_JSON2:
                return createJsonConverter(error);

            default:
                error.setError(JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_PROTOCOL, String.valueOf(getProtocolVer()));
                return null;
        }
    }

    @Override
    public void clear() {
        propertiesMap.clear();
        serviceNameIdConverter = null;
        if (Objects.nonNull(dataDictionary)) {
            dataDictionary = null;
        }
    }

    private int getProtocolVer() {
        if (!propertiesMap.containsKey(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION))
            propertiesMap.put(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2);

        return (int) propertiesMap.get(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION);
    }

    private JsonConverter createJsonConverter(JsonConverterError error) {
        JsonConverterBaseImpl result = new JsonConverterBaseImpl();

        for (Map.Entry<Integer, Object> entry : propertiesMap.entrySet()) {
            Integer propertyId = entry.getKey();
            switch (propertyId) {
                case JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS:
                    if (checkBoolean(entry, error)) {
                        result.catchUnexpectedKeys((Boolean) entry.getValue());
                    } else {
                        return null;
                    }
                    break;

                case JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_FIDS:
                    if (checkBoolean(entry, error)) {
                        result.catchUnexpectedFids((Boolean) entry.getValue());
                    } else {
                        return null;
                    }
                    break;

                case JsonConverterProperties.JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS:
                    if (checkBoolean(entry, error)) {
                        result.allowEnumDisplayStrings((Boolean) entry.getValue());
                    } else {
                        return null;
                    }
                    break;

                case JsonConverterProperties.JSON_CPC_USE_DEFAULT_DYNAMIC_QOS:
                    if (checkBoolean(entry, error)) {
                        result.useDefaultDynamicQoS((Boolean) entry.getValue());
                    } else {
                        return null;
                    }
                    break;

                case JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS:
                    if (checkBoolean(entry, error)) {
                        result.expandEnumFields((Boolean) entry.getValue());
                    } else {
                        return null;
                    }
                    break;

                case JsonConverterProperties.JSON_CPC_DEFAULT_SERVICE_ID:
                    if (checkInteger(entry, error)) {
                        result.setDefaultServiceId((Integer) entry.getValue());
                        result.setHasDefaultServiceId(true);
                    } else {
                        return null;
                    }
                    break;

                case JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION:
                    break;

                default:
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNKNOWN_PROPERTY, entry.toString() + " properties: " + propertiesMap.toString());
            }
        }
        result.setDictionary(dataDictionary);
        result.setServiceNameIdConverter(serviceNameIdConverter);
        return result;
    }

    private boolean checkBoolean(Map.Entry<Integer, Object> entry, JsonConverterError error) {
        if (!(entry.getValue() instanceof Boolean)) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNKNOWN_PROPERTY, entry.getKey() + " expected boolean");
            return false;
        }
        return true;
    }

    private boolean checkInteger(Map.Entry<Integer, Object> entry, JsonConverterError error) {
        if (!(entry.getValue() instanceof Integer)) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_UNKNOWN_PROPERTY, entry.getKey() + " expected integer");
            return false;
        }
        return true;
    }
}
