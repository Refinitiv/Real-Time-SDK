/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.itemview;

import java.util.ArrayList;
import java.util.List;

public class ServiceInfoModel {
    private String serviceName;

    private boolean isServiceUp;

    private boolean isAcceptingRequests = true;

    private List<SupportedItemDomains> supportedItemDomains = new ArrayList<>();

    public ServiceInfoModel() {}

    public ServiceInfoModel(String serviceName, List<SupportedItemDomains> supportedDomains, boolean isServiceUp, boolean isAcceptingRequests) {
        this.serviceName = serviceName;
        this.supportedItemDomains = supportedDomains;
        this.isAcceptingRequests = isAcceptingRequests;
        this.isServiceUp = isServiceUp;
    }

    public String getServiceName() {
        return serviceName;
    }

    public void setServiceName(String serviceName) {
        this.serviceName = serviceName;
    }

    public List<SupportedItemDomains> getSupportedDomains() {
        return supportedItemDomains;
    }

    public void setSupportedDomains(List<SupportedItemDomains> supportedItemDomains) {
        this.supportedItemDomains = supportedItemDomains;
    }

    public void setServiceUp(boolean serviceUp) {
        isServiceUp = serviceUp;
    }

    public void setAcceptingRequests(boolean acceptingRequests) {
        isAcceptingRequests = acceptingRequests;
    }

    public boolean addToList() {
        return isServiceUp && isAcceptingRequests;
    }

    public ServiceInfoModel copy() {
        List<SupportedItemDomains> sd = new ArrayList<>();
        sd.addAll(supportedItemDomains);
        return new ServiceInfoModel(serviceName, sd, isServiceUp, isAcceptingRequests);
    }

    @Override
    public String toString() {
        return serviceName;
    }
}
