/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.application;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model.EncryptionDataModel;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model.ProxyDataModel;

import java.util.Objects;

public class GlobalApplicationSettings {

    private ApplicationTypes applicationType;

    private ProxyDataModel proxyData;

    private EncryptionDataModel encryptionData;

    public ApplicationTypes getApplicationType() {
        return applicationType;
    }

    public void setApplicationType(ApplicationTypes applicationType) {
        this.applicationType = applicationType;
    }

    public ProxyDataModel getProxyData() {
        return proxyData;
    }

    public void setProxyData(ProxyDataModel proxyData) {
        this.proxyData = proxyData;
    }

    public EncryptionDataModel getEncryptionData() {
        return encryptionData;
    }

    public void setEncryptionData(EncryptionDataModel encryptionData) {
        this.encryptionData = encryptionData;
    }

    public boolean isEncrypted() {
        return Objects.nonNull(getEncryptionData());
    }

    public boolean isProxyUsed() {
        return Objects.nonNull(getProxyData());
    }
}
