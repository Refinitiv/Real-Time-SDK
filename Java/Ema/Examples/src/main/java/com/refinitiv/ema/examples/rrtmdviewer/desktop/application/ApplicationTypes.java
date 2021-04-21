package com.refinitiv.ema.examples.rrtmdviewer.desktop.application;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationLayouts;

public enum ApplicationTypes {
    DISCOVERED_ENDPOINT("DISCOVERED", ApplicationLayouts.DISCOVERED_ENDPOINT_SETTINGS),
    SPECIFIED_ENDPOINT("SPECIFIED", ApplicationLayouts.SPECIFIED_ENDPOINT_CONNECT);

    private String strValue;

    private ApplicationLayouts layout;

    ApplicationTypes(String strValue, ApplicationLayouts layout) {
        this.strValue = strValue;
        this.layout = layout;
    }

    public String getStrValue() {
        return strValue;
    }

    public ApplicationLayouts getLayout() {
        return layout;
    }

    public static ApplicationTypes getApplicationType(String strValue) {
        for (ApplicationTypes type : values()) {
            if (type.getStrValue().equals(strValue)) {
                return type;
            }
        }
        return null;
    }
}
