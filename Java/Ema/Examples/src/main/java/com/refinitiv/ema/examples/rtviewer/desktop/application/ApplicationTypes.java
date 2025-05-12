/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.application;

import com.refinitiv.ema.examples.rtviewer.desktop.common.ApplicationLayouts;

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
