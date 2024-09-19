/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.common;

import java.net.URL;

/**
 * Custom FX component layouts
 */
public enum ApplicationFxComponents {

    PASSWORD_EYE_COMPONENT("password_eye_component.fxml"),
    MARKET_PRICE_COMPONENT("market_price_component.fxml"),
    MARKET_BY_COMPONENT("market_by_component.fxml"),
    MARKET_BY_PRICE_COMPONENT("market_by_price_component.fxml"),
    DICTIONARY_LOADER_COMPONENT("dictionary_loader_component.fxml"),
    ERROR_DEBUG_AREA_COMPONENT("error_debug_area_component.fxml"),
    FILE_PICKER_COMPONENT("file_picker_component.fxml"),
    SCROLLABLE_TEXT_FIELD("scrollable_textfield_component.fxml"),
    EMA_CONFIG_COMPONENT("ema_config_component.fxml");

    private static final String ROOT_PATH = "/rrtmdviewer/desktop/layouts/components/";

    private String resourcePath;

    ApplicationFxComponents(String resourcePath) {
        this.resourcePath = ROOT_PATH + resourcePath;
    }

    public String getResourcePath() {
        return resourcePath;
    }

    public URL getResource() {
        return getClass().getResource(this.getResourcePath());
    }
}
