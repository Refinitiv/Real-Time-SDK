/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.common;

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
