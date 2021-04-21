package com.refinitiv.ema.examples.rrtmdviewer.desktop.common;

import java.net.URL;

public enum  ApplicationStyles {

    DISCOVERED_ENDPOINT("discovered-endpoint-style.css"),
    SPECIFIED_ENDPOINT("specified-endpoint-style.css"),
    ITEM_VIEW("item-view-style.css");

    private static final String ROOT_PATH = "/rrtmdviewer/desktop/styles/";

    private String resourcePath;

    ApplicationStyles(String resourcePath) {
        this.resourcePath = ROOT_PATH + resourcePath;
    }

    public String getResourcePath() {
        return resourcePath;
    }

    public URL getResource() {
        return getClass().getResource(this.getResourcePath());
    }
}
