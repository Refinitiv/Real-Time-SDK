/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

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
