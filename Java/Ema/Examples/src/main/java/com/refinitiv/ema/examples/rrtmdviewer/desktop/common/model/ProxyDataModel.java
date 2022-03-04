/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model;

import java.util.Objects;

public class ProxyDataModel {

    private String host;

    private String port;

    private ProxyAuthenticationDataModel proxyAuthentication;

    private ProxyDataModel(String host, String port, ProxyAuthenticationDataModel proxyAuthentication) {
        this.host = host;
        this.port = port;
        this.proxyAuthentication = proxyAuthentication;
    }

    public String getHost() {
        return host;
    }

    public String getPort() {
        return port;
    }

    public ProxyAuthenticationDataModel getProxyAuthentication() {
        return proxyAuthentication;
    }

    public boolean isAuthenticationUsed() {
        return Objects.nonNull(proxyAuthentication);
    }

    public static ProxyDataModelBuilder builder() {
        return new ProxyDataModelBuilder();
    }

    public static class ProxyDataModelBuilder {
        private String host;
        private String port;
        private ProxyAuthenticationDataModel proxyAuthentication;

        public ProxyDataModelBuilder host(String host) {
            this.host = host;
            return this;
        }

        public ProxyDataModelBuilder port(String port) {
            this.port = port;
            return this;
        }

        public ProxyDataModelBuilder useProxyAuthentication(ProxyAuthenticationDataModel proxyAuthentication) {
            this.proxyAuthentication = proxyAuthentication;
            return this;
        }

        public ProxyDataModel build() {
            return new ProxyDataModel(host, port, proxyAuthentication);
        }
    }
}
