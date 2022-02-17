package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.DataTypes;

public class JsonAnsiPageConverter extends JsonNonRWFConverter {

    JsonAnsiPageConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.ANSI_PAGE };
    }
}
