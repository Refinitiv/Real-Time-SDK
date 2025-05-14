/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.discovered_endpoint;

import java.util.List;
import java.util.Objects;

public class DiscoveredEndpointInfoModel {
    private final StringBuilder labelBuilder = new StringBuilder();
    private String endpoint;
    private String port;
    private List<String> locations;
    private String label;

    public String getEndpoint() {
        return endpoint;
    }

    public void setEndpoint(String host) {
        this.endpoint = host;
    }

    public String getPort() {
        return port;
    }

    public void setPort(String port) {
        this.port = port;
    }

    public List<String> getLocations() {
        return locations;
    }

    public void setLocations(List<String> locations) {
        this.locations = locations;
    }

    public void composeLabel() {
        labelBuilder.setLength(0);
        labelBuilder.append(endpoint).append(" ");
        int i;
        for (i = 0; i < locations.size() - 1; i++) {
            labelBuilder.append(locations.get(i));
            labelBuilder.append(",");
        }
        labelBuilder.append(locations.get(i));
        label = labelBuilder.toString();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof DiscoveredEndpointInfoModel)) return false;
        DiscoveredEndpointInfoModel that = (DiscoveredEndpointInfoModel) o;
        return label.equals(that.label);
    }

    @Override
    public int hashCode() {
        return Objects.hash(label);
    }

    @Override
    public String toString() {
        return label;
    }
}
