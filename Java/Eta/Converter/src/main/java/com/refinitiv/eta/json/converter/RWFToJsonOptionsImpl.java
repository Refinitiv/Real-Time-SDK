package com.refinitiv.eta.json.converter;

class RWFToJsonOptionsImpl implements RWFToJsonOptions {

    private int converterFlags;
    private int JsonProtocolType;
    private int majorVersion;
    private int minorVersion;

    RWFToJsonOptionsImpl() {}

    public int getConverterFlags() {
        return converterFlags;
    }

    public void setConverterFlags(int converterFlags) {
        this.converterFlags = converterFlags;
    }

    public int getJsonProtocolType() {
        return JsonProtocolType;
    }

    public void setJsonProtocolType(int jsonProtocolType) {
        JsonProtocolType = jsonProtocolType;
    }

    public int getMajorVersion() {
        return majorVersion;
    }

    public void setMajorVersion(int majorVersion) {
        this.majorVersion = majorVersion;
    }

    public int getMinorVersion() {
        return minorVersion;
    }

    public void setMinorVersion(int minorVersion) {
        this.minorVersion = minorVersion;
    }

    @Override
    public void clear() {
        converterFlags = 0;
        JsonProtocolType = JsonProtocol.JSON_JPT_UNKNOWN;
        majorVersion = 0;
        minorVersion = 0;
    }
}
