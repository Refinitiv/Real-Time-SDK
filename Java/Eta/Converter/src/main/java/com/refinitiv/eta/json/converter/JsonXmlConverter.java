package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.DataTypes;

public class JsonXmlConverter extends JsonNonRWFConverter {

    JsonXmlConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.XML };
    }
}
