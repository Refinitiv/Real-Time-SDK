/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.discovered_endpoint;

import com.refinitiv.ema.access.ServiceEndpointDiscoveryClient;
import com.refinitiv.ema.examples.rtviewer.desktop.common.OMMViewerError;
import com.refinitiv.ema.examples.rtviewer.desktop.common.model.AsyncResponseModel;

public interface DiscoveredEndpointSettingsService extends ServiceEndpointDiscoveryClient {
    void requestServiceDiscovery(DiscoveredEndpointSettingsModel discoveredEndpointSettings, AsyncResponseModel responseListener);

    void createOmmConsumer(DiscoveredEndpointSettingsModel discoveredEndpointSettingsModel,
                           ServiceEndpointDataModel serviceEndpointDataModel,
                           OMMViewerError viewerError);

    void initialize();

    void uninitialize();

    public boolean isInitialized();
}
