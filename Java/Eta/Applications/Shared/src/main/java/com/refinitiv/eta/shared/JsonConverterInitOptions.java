/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared;

import com.refinitiv.eta.json.converter.ServiceNameIdConverter;
import com.refinitiv.eta.codec.DataDictionary;

public class JsonConverterInitOptions {
    private int defaultServiceId;
    private ServiceNameIdConverter serviceNameIdConverter;
    private DataDictionary dataDictionary;
    private boolean jsonExpandedEnumFields;
    private boolean enableXmlTrace;

    public JsonConverterInitOptions(ServiceNameIdConverter serviceNameIdConverter,
                                    DataDictionary dataDictionary,
                                    boolean jsonExpandedEnumFields,
                                    boolean enableXmlTrace) {
        this.serviceNameIdConverter = serviceNameIdConverter;
        this.dataDictionary = dataDictionary;
        this.jsonExpandedEnumFields = jsonExpandedEnumFields;
        this.enableXmlTrace =  enableXmlTrace;
    }

    public JsonConverterInitOptions() {
    }

    public ServiceNameIdConverter getServiceNameIdConverter() {
        return serviceNameIdConverter;
    }

    public void setServiceNameIdConverter(ServiceNameIdConverter serviceNameIdConverter) {
        this.serviceNameIdConverter = serviceNameIdConverter;
    }

    public DataDictionary getDataDictionary() {
        return dataDictionary;
    }

    public void setDataDictionary(DataDictionary dataDictionary) {
        this.dataDictionary = dataDictionary;
    }

    public boolean isJsonExpandedEnumFields() {
        return jsonExpandedEnumFields;
    }

    public void setJsonExpandedEnumFields(boolean jsonExpandedEnumFields) {
        this.jsonExpandedEnumFields = jsonExpandedEnumFields;
    }

    public boolean isEnableXmlTrace() {
        return enableXmlTrace;
    }

    public int getDefaultServiceId() {
        return defaultServiceId;
    }

    public void setDefaultServiceId(int defaultServiceId) {
        this.defaultServiceId = defaultServiceId;
    }
}
