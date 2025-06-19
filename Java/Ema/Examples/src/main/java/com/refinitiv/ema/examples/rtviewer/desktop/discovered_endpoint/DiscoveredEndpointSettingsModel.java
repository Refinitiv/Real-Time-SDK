/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.discovered_endpoint;

import com.refinitiv.ema.examples.rtviewer.desktop.common.model.EmaConfigModel;
import com.refinitiv.ema.examples.rtviewer.desktop.common.model.EncryptionDataModel;
import com.refinitiv.ema.examples.rtviewer.desktop.common.model.ProxyDataModel;

import java.util.Objects;
import java.util.Optional;

public class DiscoveredEndpointSettingsModel {

    public static final String DEFAULT_TOKEN_SERVICE_URL_V1 = "https://api.refinitiv.com/auth/oauth2/v1/token";
    public static final String DEFAULT_TOKEN_SERVICE_URL_V2 = "https://api.refinitiv.com/auth/oauth2/v2/token";

    public static final String DEFAULT_DISCOVERY_ENDPOINT_URL = "https://api.refinitiv.com/streaming/pricing/v1/";

    public static final String DEFAULT_TOKEN_SERVICE_AUD_V2 = "https://login.ciam.refinitiv.com/as/token.oauth2";

    private String username;
    private String password;
    private String clientId;
    private String clientSecret;
    private String jwkPath;
    private String audience;

    private boolean useV1 = true;
    private boolean useClientSecret = true;

    private DiscoveredEndpointConnectionTypes connectionType;

    private EncryptionDataModel encryptionData;

    private ProxyDataModel proxyData;
    
    private ProxyDataModel restProxyData;

    private String tokenServiceUrl;

    private String discoveryEndpointUrl;

    private EmaConfigModel emaConfigModel;
    
    private boolean isTLSv12Enabled;
    
    private boolean isTLSv13Enabled;

    private DiscoveredEndpointSettingsModel(String username, String password, String clientId, DiscoveredEndpointConnectionTypes connectionType,
                                            EncryptionDataModel encryptionData, ProxyDataModel proxyData, ProxyDataModel restProxyData, String tokenServiceUrl,
                                            String discoveryEndpointUrl, EmaConfigModel emaConfigModel, boolean useV1, String clientSecret, String jwkPath, String audience, 
                                            boolean useClientSecret, boolean isTLSv12Enabled, boolean isTLSv13Enabled) {
        this.username = username;
        this.password = password;
        this.clientId = clientId;
        this.connectionType = connectionType;
        this.encryptionData = encryptionData;
        this.proxyData = proxyData;
        this.restProxyData = restProxyData;
        this.tokenServiceUrl = Optional.ofNullable(tokenServiceUrl).orElse(DEFAULT_TOKEN_SERVICE_URL_V1);
        this.discoveryEndpointUrl = Optional.ofNullable(discoveryEndpointUrl).orElse(DEFAULT_DISCOVERY_ENDPOINT_URL);
        this.emaConfigModel = emaConfigModel;
        this.useV1 = useV1;
        this.clientSecret = clientSecret;
        this.useClientSecret = useClientSecret;
        this.jwkPath = jwkPath;
        this.audience = audience;
        this.isTLSv12Enabled = isTLSv12Enabled;
        this.isTLSv13Enabled = isTLSv13Enabled;
    }

    public String getUsername() {
        return username;
    }

    public String getPassword() {
        return password;
    }

    public String getClientId() {
        return clientId;
    }

    public DiscoveredEndpointConnectionTypes getConnectionType() {
        return connectionType;
    }

    public EncryptionDataModel getEncryptionData() {
        return encryptionData;
    }

    public ProxyDataModel getProxyData() {
        return proxyData;
    }
    
    public ProxyDataModel getRestProxyData() {
    	return restProxyData;
    }

    public String getTokenServiceUrl() {
        return tokenServiceUrl;
    }

    public String getDiscoveryEndpointUrl() {
        return discoveryEndpointUrl;
    }

    public boolean isEncrypted() {
        return Objects.nonNull(getEncryptionData());
    }

    public boolean isProxyUsed() {
        return Objects.nonNull(getProxyData());
    }
    
    public boolean isRestProxyUsed() {
    	return Objects.nonNull(getRestProxyData());
    }

    public boolean isDefaultDiscovery() {
        return Objects.equals(tokenServiceUrl, DEFAULT_TOKEN_SERVICE_URL_V1)
                && Objects.equals(discoveryEndpointUrl, DEFAULT_DISCOVERY_ENDPOINT_URL);
    }

    public EmaConfigModel getEmaConfigModel() {
        return emaConfigModel;
    }

    public String getClientSecret() {
        return clientSecret;
    }

    public String getJwkPath() {
        return jwkPath;
    }

    public String getAudience() {
        return audience;
    }

    public boolean useV1() {
        return useV1;
    }

    public boolean useClientSecret() {
        return useClientSecret;
    }
    
    public boolean isTLSv12Enabled() {
    	return isTLSv12Enabled;
    }
    
    public boolean isTLSv13Enabled() {
    	return isTLSv13Enabled;
    }

    public static DiscoveredEndpointSettingsModelBuilder builder() {
        return new DiscoveredEndpointSettingsModelBuilder();
    }

    public static class DiscoveredEndpointSettingsModelBuilder {
        private String username;
        private String password;
        private String clientId;
        private DiscoveredEndpointConnectionTypes connectionType;
        private EncryptionDataModel encryptionData;
        private ProxyDataModel proxyData;
        private ProxyDataModel restProxyData;
        private String tokenServiceUrl;
        private String serviceEndpointUrl;
        private EmaConfigModel emaConfigModel;
        private String clientSecret;
        private boolean useV1;
        private String jwkPath;
        private String audience;
        private boolean useClientSecret;
        private boolean isTLSv12Enabled;
        private boolean isTLSv13Enabled;

        public DiscoveredEndpointSettingsModelBuilder clientSecret(String clientSecret) {
            this.clientSecret = clientSecret;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder jwkPath(String jwkPath) {
            this.jwkPath = jwkPath;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder audience(String audience) {
            this.audience = audience;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder useV1(boolean useV1) {
            this.useV1 = useV1;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder useClientSecret(boolean useClientSecret) {
            this.useClientSecret = useClientSecret;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder username(String username) {
            this.username = username;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder password(String password) {
            this.password = password;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder clientId(String clientId) {
            this.clientId = clientId;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder connectionType(DiscoveredEndpointConnectionTypes connectionType) {
            this.connectionType = connectionType;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder useEncryption(EncryptionDataModel encryptionDataModel) {
            this.encryptionData = encryptionDataModel;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder useProxy(ProxyDataModel proxyDataModel) {
            this.proxyData = proxyDataModel;
            return this;
        }
        
        public DiscoveredEndpointSettingsModelBuilder useRestProxy(ProxyDataModel restProxyDataModel) {
            this.restProxyData = restProxyDataModel;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder tokenServiceUrl(String tokenServiceUrl) {
            this.tokenServiceUrl = tokenServiceUrl;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder serviceEndpointUrl(String serviceEndpointUrl) {
            this.serviceEndpointUrl = serviceEndpointUrl;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder useEmaConfig(EmaConfigModel emaConfig) {
            this.emaConfigModel = emaConfig;
            return this;
        }
        
        public DiscoveredEndpointSettingsModelBuilder isTLSv12Enabled(boolean isTLSv12Enabled) {
        	this.isTLSv12Enabled = isTLSv12Enabled;
        	return this;
        }
        
        public DiscoveredEndpointSettingsModelBuilder isTLSv13Enabled(boolean isTLSv13Enabled) {
        	this.isTLSv13Enabled = isTLSv13Enabled;
        	return this;
        }

        public DiscoveredEndpointSettingsModel build() {
            return new DiscoveredEndpointSettingsModel(username, password, clientId, connectionType, encryptionData, proxyData,
                    restProxyData, tokenServiceUrl, serviceEndpointUrl, emaConfigModel, useV1, clientSecret, jwkPath, audience, useClientSecret, 
                    isTLSv12Enabled, isTLSv13Enabled);
        }
    }
}
