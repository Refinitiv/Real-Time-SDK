/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.discovered_endpoint;

import com.refinitiv.ema.examples.rtviewer.desktop.common.model.DictionaryDataModel;

import java.util.ArrayList;
import java.util.List;

public class ServiceEndpointDataModel {

    private DictionaryDataModel dictionaryData;

    private List<DiscoveredEndpointInfoModel> endpoints = new ArrayList<>();

    public DictionaryDataModel getDictionaryData() {
        return dictionaryData;
    }

    public void setDictionaryData(DictionaryDataModel dictionaryData) {
        this.dictionaryData = dictionaryData;
    }

    public List<DiscoveredEndpointInfoModel> getEndpoints() {
        return endpoints;
    }

    public void setEndpoints(List<DiscoveredEndpointInfoModel> endpoints) {
        this.endpoints = endpoints;
    }
}
