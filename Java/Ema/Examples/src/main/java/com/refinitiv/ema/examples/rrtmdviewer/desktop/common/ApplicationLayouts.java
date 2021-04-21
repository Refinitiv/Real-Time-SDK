package com.refinitiv.ema.examples.rrtmdviewer.desktop.common;

import java.net.URL;

/**
 * Main stage layouts
 */
public enum ApplicationLayouts {

    APPLICATION_SETTINGS("application_settings.fxml"),
    DISCOVERED_ENDPOINT_SETTINGS("discovered_endpoint.fxml", ApplicationStyles.DISCOVERED_ENDPOINT),
    SPECIFIED_ENDPOINT_CONNECT("specified_endpoint.fxml", ApplicationStyles.SPECIFIED_ENDPOINT),
    ITEM_VIEW("item_view.fxml", true, ApplicationStyles.SPECIFIED_ENDPOINT, ApplicationStyles.ITEM_VIEW);

    private static final String ROOT_PATH = "/rrtmdviewer/desktop/layouts/";

    private String resourcePath;

    private ApplicationStyles[] styles;

    private boolean lazyInit;

    ApplicationLayouts(String resourcePath, ApplicationStyles... styles) {
        this(resourcePath, false, styles);
    }

    ApplicationLayouts(String resourcePath, boolean lazyInit, ApplicationStyles... styles) {
        this.resourcePath = ROOT_PATH + resourcePath;
        this.lazyInit = lazyInit;
        this.styles = styles;
    }

    public boolean isLazyInit() {
        return lazyInit;
    }

    public String getResourcePath() {
        return resourcePath;
    }

    public URL getResource() {
        return getClass().getResource(this.getResourcePath());
    }

    public ApplicationStyles[] getStyles() {
        return styles;
    }
}
