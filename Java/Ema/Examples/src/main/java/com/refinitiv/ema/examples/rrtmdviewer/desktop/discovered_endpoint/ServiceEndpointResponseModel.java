package com.refinitiv.ema.examples.rrtmdviewer.desktop.discovered_endpoint;

import java.util.List;

public class ServiceEndpointResponseModel {
    private final List<DiscoveredEndpointInfoModel> responseData;

    public ServiceEndpointResponseModel(List<DiscoveredEndpointInfoModel> responseData) {
        this.responseData = responseData;
    }

    public List<DiscoveredEndpointInfoModel> getResponseData() {
        return responseData;
    }
}
