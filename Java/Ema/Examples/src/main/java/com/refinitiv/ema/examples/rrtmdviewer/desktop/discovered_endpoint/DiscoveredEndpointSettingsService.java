package com.refinitiv.ema.examples.rrtmdviewer.desktop.discovered_endpoint;

import com.refinitiv.ema.access.ServiceEndpointDiscoveryClient;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.OMMViewerError;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model.AsyncResponseModel;

public interface DiscoveredEndpointSettingsService extends ServiceEndpointDiscoveryClient {
    void requestServiceDiscovery(DiscoveredEndpointSettingsModel discoveredEndpointSettings, AsyncResponseModel responseListener);

    void createOmmConsumer(DiscoveredEndpointSettingsModel discoveredEndpointSettingsModel,
                           ServiceEndpointDataModel serviceEndpointDataModel,
                           OMMViewerError viewerError);

    void uninitialize();
}
