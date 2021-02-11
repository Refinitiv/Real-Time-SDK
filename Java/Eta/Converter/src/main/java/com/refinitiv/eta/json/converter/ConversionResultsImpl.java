package com.refinitiv.eta.json.converter;

class ConversionResultsImpl implements ConversionResults {

    private int length;

    ConversionResultsImpl() {}

    @Override
    public int getLength() {
        return length;
    }

    @Override
    public void setLength(int value) {
        this.length = value;
    }
}
