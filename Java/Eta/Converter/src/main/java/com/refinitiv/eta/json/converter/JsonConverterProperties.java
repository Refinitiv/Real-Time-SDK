/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

public class JsonConverterProperties {

    /**
     * Set a dictionary list on the converter.  (value: RsslJsonDictionaryListProperty).
     */
    public final static int JSON_CPC_DICTIONARY_LIST = 2;

    /**
     * (Simplified JSON) Set a default service ID to use when converting requests from JSON to RWF.
     * If Request.Key.Service is not present, this ID will be used. (value: RsslUInt16)
     */
    public final static int JSON_CPC_DEFAULT_SERVICE_ID = 3;

    /**
     * (Simplified JSON) When converting from RWF to JSON, add a QoS range on requests that
     * do not specify a Qos (value: RsslBool).
     */
    public final static int JSON_CPC_USE_DEFAULT_DYNAMIC_QOS = 4;

    /**
     * (Simplified JSON) Expand enumerated values in field entries to their display values.
     * Dictionary must have enumerations loaded (value: RsslBool).
     */
    public final static int JSON_CPC_EXPAND_ENUM_FIELDS = 5;

    /**
     * (Simplified JSON) When converting from JSON to RWF, catch unknown JSON keys.  (value: RsslBool).
     */
    public final static int JSON_CPC_CATCH_UNKNOWN_JSON_KEYS = 7;

    /**
     * (Simplified JSON) When converting from JSON to RWF, catch unknown FIDS.  (value: RsslBool).
     */
    public final static int JSON_CPC_CATCH_UNKNOWN_JSON_FIDS = 8;

    /**
     * (Simplified JSON) When fields with enum display strings are encountered in JSON, treat them as blank when
     * converting to RWF instead of returning an error (value: RsslBool).
     */
    public final static int JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS = 9;

    public final static int JSON_CPC_PROTOCOL_VERSION = 10;

    private JsonConverterProperties() {
        throw new IllegalAccessError();
    }
}
