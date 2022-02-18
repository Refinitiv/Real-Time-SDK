package com.refinitiv.ema.examples.rrtmdviewer.desktop.discovered_endpoint;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model.EmaConfigModel;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model.EncryptionDataModel;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model.ProxyDataModel;

import java.util.Objects;
import java.util.Optional;

public class DiscoveredEndpointSettingsModel {

    public static final String DEFAULT_TOKEN_SERVICE_URL_V1 = "https://api.refinitiv.com/auth/oauth2/v1/token";
    public static final String DEFAULT_TOKEN_SERVICE_URL_V2 = "https://api.ppe.refinitiv.com/auth/oauth2/v2/token";

    public static final String DEFAULT_DISCOVERY_ENDPOINT_URL = "https://api.refinitiv.com/streaming/pricing/v1/";

    private String username;
    private String password;
    private String clientId;
    private String clientSecret;
    private String jwkPath;

    private boolean useV1 = true;
    private boolean useClientSecret = true;

    private DiscoveredEndpointConnectionTypes connectionType;

    private EncryptionDataModel encryptionData;

    private ProxyDataModel proxyData;

    private String tokenServiceUrl;

    private String discoveryEndpointUrl;

    private EmaConfigModel emaConfigModel;

    private DiscoveredEndpointSettingsModel(String username, String password, String clientId, DiscoveredEndpointConnectionTypes connectionType,
                                            EncryptionDataModel encryptionData, ProxyDataModel proxyData, String tokenServiceUrl,
                                            String discoveryEndpointUrl, EmaConfigModel emaConfigModel, boolean useV1, String clientSecret, String jwkPath, boolean useClientSecret) {
        this.username = username;
        this.password = password;
        this.clientId = clientId;
        this.connectionType = connectionType;
        this.encryptionData = encryptionData;
        this.proxyData = proxyData;
        this.tokenServiceUrl = Optional.ofNullable(tokenServiceUrl).orElse(DEFAULT_TOKEN_SERVICE_URL_V1);
        this.discoveryEndpointUrl = Optional.ofNullable(discoveryEndpointUrl).orElse(DEFAULT_DISCOVERY_ENDPOINT_URL);
        this.emaConfigModel = emaConfigModel;
        this.useV1 = useV1;
        this.clientSecret = clientSecret;
        this.jwkPath = jwkPath;
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

    public boolean useV1() {
        return useV1;
    }

    public boolean useClientSecret() {
        return useClientSecret;
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
        private String tokenServiceUrl;
        private String serviceEndpointUrl;
        private EmaConfigModel emaConfigModel;
        private String clientSecret;
        private boolean useV1;
        private String jwkPath;
        private boolean useClientSecret;

        public DiscoveredEndpointSettingsModelBuilder clientSecret(String clientSecret) {
            this.clientSecret = clientSecret;
            return this;
        }

        public DiscoveredEndpointSettingsModelBuilder jwkPath(String jwkPath) {
            this.jwkPath = jwkPath;
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

        public DiscoveredEndpointSettingsModel build() {
            return new DiscoveredEndpointSettingsModel(username, password, clientId, connectionType, encryptionData, proxyData,
                    tokenServiceUrl, serviceEndpointUrl, emaConfigModel, useV1, clientSecret, jwkPath, useClientSecret);
        }
    }
}
