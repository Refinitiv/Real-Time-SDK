package com.refinitiv.eta.json.converter;

class ParseJsonOptionsImpl implements ParseJsonOptions {

    private int converterFlags;
    private int protocolType;

    ParseJsonOptionsImpl() {}

    @Override
    public void clear() {
        converterFlags = ParseJsonFlags.JSON_PJF_NONE;
        protocolType = JsonProtocol.JSON_JPT_UNKNOWN;
    }

    public int getConverterFlags() {
        return converterFlags;
    }

    public void setConverterFlags(int converterFlags) {
        this.converterFlags = converterFlags;
    }

    public int getProtocolType() {
        return protocolType;
    }

    public void setProtocolType(int protocolType) {
        this.protocolType = protocolType;
    }
}
