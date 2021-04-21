package com.refinitiv.ema.examples.rrtmdviewer.desktop.discovered_endpoint;

import java.util.List;

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
    public String toString() {
        return label;
    }
}
