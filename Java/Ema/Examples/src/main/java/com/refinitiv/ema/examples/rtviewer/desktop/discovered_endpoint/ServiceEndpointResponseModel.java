/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.discovered_endpoint;

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
