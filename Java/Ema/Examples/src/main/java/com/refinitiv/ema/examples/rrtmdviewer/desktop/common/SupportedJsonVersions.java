package com.refinitiv.ema.examples.rrtmdviewer.desktop.common;

public enum SupportedJsonVersions {

    TR_JSON("tr_json2"),
    RSSL_JSON("rssl.json.v2");

    private String textLabel;

    SupportedJsonVersions(String textLabel) {
        this.textLabel = textLabel;
    }

    @Override
    public String toString() {
        return textLabel;
    }
}
