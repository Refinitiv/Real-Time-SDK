/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.discovered_endpoint;

import com.refinitiv.ema.access.ServiceEndpointDiscoveryOption;

public enum DiscoveredEndpointConnectionTypes {

    ENCRYPTED_SOCKET("Encrypted-Socket",
            ServiceEndpointDiscoveryOption.TransportProtocol.TCP
    ),

    ENCRYPTED_WEBSOCKET("Encrypted-WebSocket",
            ServiceEndpointDiscoveryOption.TransportProtocol.WEB_SOCKET
    );

    private final String textLabel;

    private final int transportProtocol;

    @Override
    public String toString() {
        return textLabel;
    }

    public int getTransportProtocol() {
        return transportProtocol;
    }

    DiscoveredEndpointConnectionTypes(String textLabel, int transportProtocol) {
        this.textLabel = textLabel;
        this.transportProtocol = transportProtocol;
    }
}
