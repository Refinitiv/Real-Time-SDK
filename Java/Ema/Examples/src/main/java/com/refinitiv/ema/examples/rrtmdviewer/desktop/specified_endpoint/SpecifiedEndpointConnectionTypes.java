/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.specified_endpoint;


public enum SpecifiedEndpointConnectionTypes {
    SOCKET("Socket"),
    WEBSOCKET("WebSocket"),
    ENCRYPTED_SOCKET("Encrypted-Socket"),
    ENCRYPTED_WEBSOCKET("Encrypted-WebSocket");

    private String textLabel;

    @Override
    public String toString() {
        return textLabel;
    }

    SpecifiedEndpointConnectionTypes(String textLabel) {
        this.textLabel = textLabel;
    }
}
